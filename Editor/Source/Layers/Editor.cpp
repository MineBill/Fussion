#include "EditorPCH.h"
#include "Editor.h"
#include "EditorUI.h"
#include "EditorWindows/AssetWindows/MaterialWindow.h"
#include "EditorWindows/AssetWindows/Texture2DWindow.h"
#include "EditorWindows/AssetRegistryViewer.h"

#include <Fussion/Input/Input.h>
#include <Fussion/Core/Application.h>
#include <Fussion/Scene/Components/BaseComponents.h>
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Events/ApplicationEvents.h>
#include <Fussion/Events/KeyboardEvents.h>
#include <Fussion/OS/FileSystem.h>
#include <Fussion/Scripting/ScriptingEngine.h>
#include <Fussion/Scene/Components/Camera.h>
#include <Fussion/Serialization/JsonSerializer.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <tracy/Tracy.hpp>
#include <ranges>

Editor* Editor::s_EditorInstance = nullptr;
AssetPicker Editor::GenericAssetPicker{};

using namespace Fussion;

Editor::Editor()
{
    VERIFY(s_EditorInstance == nullptr, "EditorLayer already exists!")
    s_EditorInstance = this;
}

Editor::~Editor() = default;

void Editor::OnStart()
{
    ZoneScoped;
    m_ViewportWindow = MakePtr<ViewportWindow>(this);
    m_InspectorWindow = MakePtr<InspectorWindow>(this);
    m_ConsoleWindow = MakePtr<ConsoleWindow>(this);
    m_SceneWindow = MakePtr<SceneTreeWindow>(this);
    m_ScriptsInspector = MakePtr<ScriptsInspector>(this);
    m_ContentBrowser = MakePtr<ContentBrowser>(this);
    m_AssetRegistryViewer = MakePtr<AssetRegistryViewer>(this);

    ScriptingEngine::Get().CompileGameAssembly(Project::GetScriptsFolder());
    FileSystem::WriteEntireFile(Project::GetScriptsFolder() / "as.predefined", ScriptingEngine::Get().DumpCurrentTypes().str());

    m_Watcher = FileWatcher::Create(Project::GetScriptsFolder());
    m_Watcher->RegisterListener([](std::filesystem::path const& path, FileWatcher::EventType type) {
        using namespace std::chrono_literals;
        (void)path;
        (void)type;

        // Wait a bit for the file lock to be released.
        std::this_thread::sleep_for(100ms);
        ScriptingEngine::Get().CompileGameAssembly(Project::GetScriptsFolder());
    });
    m_Watcher->Start();

    ImGui::LoadIniSettingsFromDisk("Assets/EditorLayout.ini");

    m_Camera.Resize(Application::Instance()->GetWindow().GetSize());
    m_Camera.Position = Vector3(0, 3, 5);
    m_SceneRenderer.Init();

    OnViewportResized(Vector2(300, 300));

    m_ViewportWindow->OnStart();
    m_InspectorWindow->OnStart();
    m_ConsoleWindow->OnStart();
    m_SceneWindow->OnStart();
    m_ContentBrowser->OnStart();

    m_AssetRegistryViewer->OnStart();
    m_AssetRegistryViewer->Hide();

    m_ScriptsInspector->OnStart();
    m_ScriptsInspector->Hide();

    OnBeginPlay += [this] {
        LOG_DEBUG("On Begin Play");
        auto meta = Project::GetAssetManager()->GetMetadata(m_ActiveScene->GetHandle());

        JsonDeserializer ds(*FileSystem::ReadEntireFile(Project::GetAssetsFolder() / meta.Path));
        m_PlayScene = MakeRef<Scene>();
        m_PlayScene->Deserialize(ds);

        if (m_PlayScene)
            m_PlayScene->OnStart();
    };

    OnStopPlay += [this] {
        LOG_DEBUG("On Stop Play");
        if (m_PlayScene) {
            m_PlayScene = nullptr;
        }
    };

    OnPaused += [] {
        LOG_DEBUG("On Paused");
    };

    OnResumePlay += [] {
        LOG_DEBUG("On Resume Play");
    };
}

void Editor::OnEnable() {}

void Editor::OnDisable() {}

void Editor::Save() const
{
    Project::Save();

    if (m_State == PlayState::Editing && m_ActiveScene != nullptr) {
        LOG_DEBUGF("Saving scene {} to {}", m_ActiveScene->Name(), m_ActiveScenePath);
        JsonSerializer js;
        js.Initialize();

        m_ActiveScene->Serialize(js);

        auto path = Project::GetAssetsFolder() / m_ActiveScenePath;
        FileSystem::WriteEntireFile(path, js.ToString());

        m_ActiveScene->SetDirty(false);
    }
}

void Editor::OpenAsset(AssetHandle handle)
{
    if (m_AssetWindows.contains(handle))
        return;

    auto assman = Project::GetAssetManager();
    auto meta = assman->GetMetadata(handle);
    if (!meta.IsValid())
        return;

    switch (meta.Type) {
    case AssetType::PbrMaterial:
        m_AssetWindows[handle] = MakePtr<MaterialWindow>(handle);
        break;
    case AssetType::Texture2D:
        m_AssetWindows[handle] = MakePtr<Texture2DWindow>(handle);
        break;
    default:
        break;
    }
}

void Editor::OnUpdate(f32 delta)
{
    ZoneScoped;

    GenericAssetPicker.Update();

    switch (m_State) {
    case PlayState::Editing: {
        m_Camera.SetFocus(m_ViewportWindow->IsFocused());
        m_Camera.OnUpdate(delta);
        if (m_ActiveScene) {
            m_ActiveScene->Tick();
            m_ActiveScene->OnDebugDraw(DebugDrawContext);
        }
    }
    break;
    case PlayState::Playing: {
        if (m_Detached) {
            m_Camera.SetFocus(m_ViewportWindow->IsFocused());
            m_Camera.OnUpdate(delta);
        }
        if (m_PlayScene) {
            m_PlayScene->OnUpdate(delta);
            m_PlayScene->OnDebugDraw(DebugDrawContext);
        }
    }
    break;
    case PlayState::Paused: {}
    break;
    }

    ImGui::DockSpaceOverViewport();

    static bool show_demo_window = false;
    ImGui::BeginMainMenuBar();
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("New..")) {
                if (ImGui::MenuItem("Create Scene")) {
                    ChangeScene(Project::GetAssetManager()->CreateAsset<Scene>("TestScene.fsn"));
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (!m_ActiveScene)
                ImGui::BeginDisabled();

            if (ImGui::MenuItem("Save..", "Ctrl+S")) {
                Save();
            }

            if (!m_ActiveScene)
                ImGui::EndDisabled();

            ImGui::Separator();

            if (ImGui::MenuItem("Quit")) {}

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {
            if (ImGui::MenuItem("Scripts Inspector")) {
                m_ScriptsInspector->Show();
            }
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("Shadow Map")) {
                    m_ShadowMapDisplayOpened = !m_ShadowMapDisplayOpened;
                }

                if (ImGui::MenuItem("Asset Registry")) {
                    m_AssetRegistryViewer->Toggle();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Extra")) {
            ImGui::Checkbox("Demo Window", &show_demo_window);
            if (ImGui::MenuItem("Save Layout")) {
                ImGui::SaveIniSettingsToDisk("Assets/EditorLayout.ini");
            }

            ImGui::EndMenu();
        }
    }
    ImGui::EndMainMenuBar();

    auto flags =
        ImGuiDockNodeFlags_NoTabBar |
        ImGuiDockNodeFlags_HiddenTabBar |
        ImGuiDockNodeFlags_NoDockingOverMe |
        ImGuiDockNodeFlags_NoResize |
        ImGuiDockNodeFlags_NoDockingOverOther;

    ImGuiWindowClass klass;
    klass.DockNodeFlagsOverrideSet = flags;

    ImGui::SetNextWindowClass(&klass);

    auto state = m_State;
    if (state == PlayState::Playing) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Color::Yellow);
    }
    EUI::Window("##toolbar", [this] {
        constexpr auto y = 4;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2{ 0, y });
        defer(ImGui::PopStyleVar());
        auto height = ImGui::GetWindowHeight() - y * 2;
        auto pos = ImGui::GetWindowContentRegionMax().x * 0.5f - 3.f * height * 0.5f;
        ImGui::SetCursorPosX(pos);
        ImGui::SetCursorPosY(y + 1);

        auto list = ImGui::GetWindowDrawList();
        list->ChannelsSplit(2);
        list->ChannelsSetCurrent(1);

        auto& style = EditorStyle::GetStyle();
        EUI::ImageButton(style.EditorIcons[EditorIcon::Play], [this] {
            SetPlayState(PlayState::Playing);
        }, { .Size = Vector2{ height, height }, .Disabled = m_ActiveScene == nullptr || m_State == PlayState::Playing });

        auto min = ImGui::GetItemRectMin();

        ImGui::SetItemTooltip("Begin Play mode. This will simulate the game with the in-game camera.");

        ImGui::SameLine();

        EUI::ImageButton(style.EditorIcons[EditorIcon::Stop], [this] {
            SetPlayState(PlayState::Editing);
        }, { .Size = Vector2{ height, height }, .Disabled = m_State != PlayState::Playing });

        ImGui::SameLine();

        EUI::ImageButton(style.EditorIcons[EditorIcon::Pause], [this] {
            SetPlayState(PlayState::Paused);
        }, { .Size = Vector2{ height, height }, .Disabled = m_State != PlayState::Playing });

        ImGui::SameLine();

        EUI::ImageButton(style.EditorIcons[EditorIcon::Dots], [this] {
            ImGui::OpenPopup("Toolbar::Options");
        }, { .Size = Vector2{ height, height }, .Disabled = m_State != PlayState::Playing });

        EUI::Popup("Toolbar::Options", [&] {
            if (ImGui::MenuItem("Detach")) {
                m_Detached = !m_Detached;
            }
        });

        ImGui::SetItemTooltip("Pause");

        auto max = ImGui::GetItemRectMax();

        list->ChannelsSetCurrent(0);

        Color color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        color = color.Lighten(0.2f);

        list->AddRectFilled(min, max, color.ToABGR(), 3);

        list->ChannelsMerge();
    }, { .Flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize, .UseChild = false });
    if (state == PlayState::Playing) {
        ImGui::PopStyleColor();
    }

    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // TODO: Is there a better place to put this?
    if (m_ShadowMapDisplayOpened) {
        EUI::Window("Shadow Map", [&] {
            EUI::Property("Cascade Index", &m_SceneRenderer.RenderDebugOptions.CascadeIndex, EUI::PropTypeRange{ .Min = 0.0f, .Max = ShadowCascades - 1 });
            ImGui::Image(IMGUI_IMAGE(m_SceneRenderer.GetShadowDebugFrameBuffer()->GetColorAttachment(0)), ImGui::GetContentRegionAvail());
        }, { .Opened = &m_ShadowMapDisplayOpened });
    }

    m_ViewportWindow->OnDraw();
    m_InspectorWindow->OnDraw();
    m_ConsoleWindow->OnDraw();
    m_SceneWindow->OnDraw();
    m_ScriptsInspector->OnDraw();
    m_ContentBrowser->OnDraw();

    if (m_AssetRegistryViewer->IsVisible())
        m_AssetRegistryViewer->OnDraw();

    for (auto const& asset_window : m_AssetWindows | std::views::values) {
        asset_window->Draw(delta);
    }

    std::erase_if(m_AssetWindows, [](auto const& pair) {
        return !pair.second->IsOpen();
    });

    Undo.Commit();

    Project::GetAssetManager()->CheckForLoadedAssets();
}

void Editor::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<OnKeyPressed>([this](OnKeyPressed const& e) -> bool {
        if (e.Key == Keys::Z && e.Mods.Test(KeyMod::Control)) {
            Undo.Undo();
        }

        if (e.Key == Keys::Y && e.Mods.Test(KeyMod::Control)) {
            Undo.Redo();
        }

        if (e.Key == Keys::S && e.Mods.Test(KeyMod::Control)) {
            Save();
        }
        return false;
    });

    dispatcher.Dispatch<WindowCloseRequest>([this](WindowCloseRequest const&) {
        if (m_ActiveScene != nullptr && m_ActiveScene->IsDirty()) {
            Dialogs::MessageBox data{};
            data.Message = "The current scene has unsaved modifications. Are you sure you want to quit?";
            data.Action = Dialogs::MessageAction::OkCancel;
            switch (Dialogs::ShowMessageBox(data)) {
            case Dialogs::MessageButton::Ok:
            case Dialogs::MessageButton::Yes:
                Application::Instance()->Quit();
                break;
            case Dialogs::MessageButton::No:
                break;
            case Dialogs::MessageButton::Cancel:
                break;
            }
        } else {
            Application::Instance()->Quit();
        }
        return true;
    });

    m_Camera.HandleEvent(event);

    m_ViewportWindow->OnEvent(event);
    m_InspectorWindow->OnEvent(event);
    m_ConsoleWindow->OnEvent(event);
    m_SceneWindow->OnEvent(event);
    m_ScriptsInspector->OnEvent(event);
    m_ContentBrowser->OnEvent(event);
}

void Editor::OnDraw(Ref<RHI::CommandBuffer> const& cmd)
{
    auto RenderEditorView = [&](Ref<Scene> const& scene) {
        m_SceneRenderer.Render(cmd, {
            .Camera = RenderCamera{
                .Perspective = m_Camera.GetPerspective(),
                .View = m_Camera.GetView(),
                .Position = m_Camera.Position,
                .Near = m_Camera.Near,
                .Far = m_Camera.Far,
                .Direction = m_Camera.GetDirection(),
            },
            .Scene = scene.get(),
        }, false);
    };
    auto RenderGameView = [&](Camera const& camera) {
        auto entity = camera.GetOwner();
        m_SceneRenderer.Render(cmd, {
            .Camera = RenderCamera{
                .Perspective = camera.GetPerspective(),
                .View = entity->Transform.GetCameraMatrix(),
                .Position = entity->Transform.Position,
                .Near = camera.Near,
                .Far = camera.Far,
                .Direction = entity->Transform.GetForward(),
            },
            .Scene = m_PlayScene.get(),
        }, true);
    };
    switch (m_State) {
    case PlayState::Editing: {
        RenderEditorView(m_ActiveScene);
    }
    break;
    case PlayState::Playing:
    case PlayState::Paused: {
        auto camera = m_PlayScene->FindFirstComponent<Camera>();

        RenderGameView(*camera.get());
    }
    break;
    case PlayState::Detached: {
        auto camera = m_PlayScene->FindFirstComponent<Camera>();

        RenderEditorView(m_PlayScene);
    }
    break;
    }

}

void Editor::SetPlayState(PlayState new_state)
{
    ZoneScoped;
    switch (m_State) {
    case PlayState::Editing:
        switch (new_state) {
        case PlayState::Playing:
            OnBeginPlay.Fire();
            break;
        // noop
        case PlayState::Editing:
        case PlayState::Paused:
        case PlayState::Detached:
            return;
        }
        break;
    case PlayState::Playing:
        switch (new_state) {
        case PlayState::Editing:
            OnStopPlay.Fire();
            break;
        case PlayState::Playing:
            // noop
            return;
        case PlayState::Paused:
            OnPaused.Fire();
            break;
        case PlayState::Detached:
            break;
        }
        break;
    case PlayState::Paused:
        switch (new_state) {
        case PlayState::Editing:
            OnStopPlay.Fire();
            break;
        case PlayState::Playing:
            OnResumePlay.Fire();
            break;
        case PlayState::Detached:
            break;
        case PlayState::Paused:
            // noop
            return;
        }
        break;
    case PlayState::Detached:
        switch (new_state) {
        case PlayState::Editing:
            break;
        case PlayState::Playing:
            break;
        case PlayState::Paused:
            break;
        case PlayState::Detached:
            // noop
            return;
        }
        break;
    }
    m_State = new_state;
}

void Editor::OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc)
{
    m_LogEntries.push_back(LogEntry{
        level,
        std::string(message),
        loc,
    });
}

void Editor::ChangeScene(AssetRef<Scene> scene)
{
    auto LoadScene = [&scene] {
        s_EditorInstance->m_SceneWindow->ClearSelection();

        auto meta = Project::GetAssetManager()->GetMetadata(scene.Handle());

        if (auto scene_json = FileSystem::ReadEntireFile(Project::GetAssetsFolder() / meta.Path)) {
            JsonDeserializer ds(*scene_json);

            auto scene_asset = MakeRef<Scene>();
            scene_asset->Deserialize(ds);
            scene_asset->SetHandle(scene.Handle());

            s_EditorInstance->m_ActiveScenePath = meta.Path;
            s_EditorInstance->m_ActiveScene = scene_asset;
        }
    };
    if (s_EditorInstance->m_ActiveScene != nullptr && s_EditorInstance->m_ActiveScene->IsDirty()) {
        Dialogs::MessageBox data{};
        data.Message = "The current scene has unsaved modifications. Are you sure you want to discard them? Selecting 'No' will save the current scene and load the new one.";
        data.Action = Dialogs::MessageAction::YesNoCancel;
        switch (Dialogs::ShowMessageBox(data)) {
        case Dialogs::MessageButton::No: { s_EditorInstance->Save(); }
        case Dialogs::MessageButton::Yes: {
            LoadScene();
        }
        break;
        case Dialogs::MessageButton::Cancel:
            break;
        default:
            UNIMPLEMENTED;
        }
    } else {
        LoadScene();
    }
}

void Editor::OnViewportResized(Vector2 const& new_size)
{
    ZoneScoped;

    s_EditorInstance->m_Camera.Resize(new_size);
    s_EditorInstance->m_SceneRenderer.Resize(new_size);
}
