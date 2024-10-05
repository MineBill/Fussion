#include "Editor.h"

#include "EditorPCH.h"
#include "EditorUI.h"
#include "EditorWindows/AssetRegistryViewer.h"
#include "EditorWindows/AssetWindows/MaterialWindow.h"
#include "EditorWindows/AssetWindows/Texture2DWindow.h"
#include "EditorWindows/RendererReport.h"
#include "Fussion/Math/BoundingBox.h"

#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Core/Application.h>
#include <Fussion/Events/ApplicationEvents.h>
#include <Fussion/Events/KeyboardEvents.h>
#include <Fussion/Input/Input.h>
#include <Fussion/OS/FileSystem.h>
#include <Fussion/Scene/Components/BaseComponents.h>
#include <Fussion/Scene/Components/Camera.h>
#include <Fussion/Scripting/ScriptingEngine.h>
#include <Fussion/Serialization/JsonSerializer.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>
#include <tracy/Tracy.hpp>

Editor* Editor::s_EditorInstance = nullptr;
AssetPicker Editor::GenericAssetPicker {};

using namespace Fussion;

Editor::Editor()
{
    VERIFY(s_EditorInstance == nullptr, "EditorLayer already exists!")
    s_EditorInstance = this;
}

Editor::~Editor() = default;

void Editor::OnStart()
{
    Application::Self()->GetWindow().SetTitle(fmt::format("Fussion - {}", Project::Name()));
    Application::Self()->GetWindow().Maximize();

    ZoneScoped;
    m_ViewportWindow = MakePtr<ViewportWindow>(this);
    m_InspectorWindow = MakePtr<InspectorWindow>(this);
    m_ConsoleWindow = MakePtr<ConsoleWindow>(this);
    m_SceneWindow = MakePtr<SceneTreeWindow>(this);
    m_ScriptsInspector = MakePtr<ScriptsInspector>(this);
    m_ContentBrowser = MakePtr<ContentBrowserWindow>(this);
    m_AssetRegistryViewer = MakePtr<AssetRegistryViewer>(this);
    m_RendererReport = MakePtr<RendererReport>(this);

    ScriptingEngine::Self().CompileGameAssembly(Project::ScriptsFolderPath());
    FileSystem::WriteEntireFile(Project::ScriptsFolderPath() / "as.predefined", ScriptingEngine::Self().DumpCurrentTypes().str());

    m_Watcher = FileWatcher::Create(Project::ScriptsFolderPath());
    m_Watcher->AddListener([](std::filesystem::path const& path, FileWatcher::EventType type) {
        using namespace std::chrono_literals;
        (void)path;
        (void)type;

        // Wait a bit for the file lock to be released.
        std::this_thread::sleep_for(100ms);
        ScriptingEngine::Self().CompileGameAssembly(Project::ScriptsFolderPath());
    });
    m_Watcher->Start();

    ImGui::LoadIniSettingsFromDisk("Assets/EditorLayout.ini");

    m_Camera.Resize(Application::Self()->GetWindow().Size());
    m_Camera.Position = Vector3(0, 3, 5);
    m_SceneRenderer.init();

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
        auto meta = Project::AssetManager()->GetMetadata(m_ActiveScene->GetHandle());

        JsonDeserializer ds(*FileSystem::ReadEntireFile(Project::AssetsFolderPath() / meta.Path));
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

void Editor::OnEnable() { }

void Editor::OnDisable() { }

void Editor::Save() const
{
    for (auto& asd : m_AssetWindows) {
        asd.second->OnSave();
    }

    Project::Save();

    if (m_PlayState == PlayState::Editing && m_ActiveScene != nullptr) {
        LOG_DEBUGF("Saving scene {} to {}", m_ActiveScene->GetName(), m_ActiveScenePath);
        JsonSerializer js;
        js.Initialize();

        m_ActiveScene->Serialize(js);

        auto path = Project::AssetsFolderPath() / m_ActiveScenePath;
        FileSystem::WriteEntireFile(path, js.ToString());

        m_ActiveScene->SetDirty(false);
    }
}

void Editor::OpenAsset(AssetHandle handle)
{
    if (m_AssetWindows.contains(handle))
        return;

    auto assman = Project::AssetManager();
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

    switch (m_PlayState) {
    case PlayState::Editing: {
        m_Camera.SetFocus(m_ViewportWindow->IsFocused());
        m_Camera.OnUpdate(delta);
        if (m_ActiveScene) {
            m_ActiveScene->Tick();
            m_ActiveScene->OnDebugDraw(DebugDrawContext);
        }
    } break;
    case PlayState::Playing: {
        if (m_Detached) {
            m_Camera.SetFocus(m_ViewportWindow->IsFocused());
            m_Camera.OnUpdate(delta);
        }
        if (m_PlayScene) {
            m_PlayScene->OnUpdate(delta);
            m_PlayScene->OnDebugDraw(DebugDrawContext);
        }
    } break;
    case PlayState::Paused: {
    }
    case PlayState::Detached: {
    } break;
    }

    ImGui::DockSpaceOverViewport();

    static bool show_demo_window = false;
    ImGui::BeginMainMenuBar();
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("New..")) {
                if (ImGui::MenuItem("Create Scene")) {
                    ChangeScene(Project::AssetManager()->CreateAsset<Scene>("TestScene.fsn"));
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

            if (ImGui::MenuItem("Quit")) { }

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

    auto flags = ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoDockingOverMe | ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoDockingOverOther;

    ImGuiWindowClass klass;
    klass.DockNodeFlagsOverrideSet = flags;

    ImGui::SetNextWindowClass(&klass);

    auto state = m_PlayState;
    if (state == PlayState::Playing) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, Color::Yellow);
    }
    EUI::Window("##toolbar", [this] {
        constexpr auto y = 4;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2 { 0, y });
        defer(ImGui::PopStyleVar());
        auto height = ImGui::GetWindowHeight() - y * 2;
        auto pos = ImGui::GetWindowContentRegionMax().x * 0.5f - 3.f * height * 0.5f;
        ImGui::SetCursorPosX(pos);
        ImGui::SetCursorPosY(y + 1);

        auto list = ImGui::GetWindowDrawList();
        list->ChannelsSplit(2);
        list->ChannelsSetCurrent(1);

        auto& style = EditorStyle::Style();
        EUI::ImageButton(style.EditorIcons[EditorIcon::Play], [this] {
            SetPlayState(PlayState::Playing);
        },
            { .size = Vector2 { height, height }, .disabled = m_ActiveScene == nullptr || m_PlayState == PlayState::Playing });

        auto min = ImGui::GetItemRectMin();

        ImGui::SetItemTooltip("Begin Play mode. This will simulate the game with the in-game camera.");

        ImGui::SameLine();

        EUI::ImageButton(style.EditorIcons[EditorIcon::Stop], [this] {
            SetPlayState(PlayState::Editing);
        },
            { .size = Vector2 { height, height }, .disabled = m_PlayState != PlayState::Playing });

        ImGui::SameLine();

        EUI::ImageButton(style.EditorIcons[EditorIcon::Pause], [this] {
            SetPlayState(PlayState::Paused);
        },
            { .size = Vector2 { height, height }, .disabled = m_PlayState != PlayState::Playing });

        ImGui::SameLine();

        EUI::ImageButton(style.EditorIcons[EditorIcon::Dots], [] {
            ImGui::OpenPopup("Toolbar::Options");
        },
            { .size = Vector2 { height, height }, .disabled = m_PlayState != PlayState::Playing });

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
    },
        { .flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize, .use_child = false });
    if (state == PlayState::Playing) {
        ImGui::PopStyleColor();
    }

    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // TODO: Is there a better place to put this?
    // if (m_ShadowMapDisplayOpened) {
    //     EUI::Window("Shadow Map", [&] {
    //         EUI::Property("Cascade Index", &m_SceneRenderer.RenderDebugOptions.CascadeIndex, EUI::PropTypeRange{ .Min = 0.0f, .Max = ShadowCascades - 1 });
    //         ImGui::Image(IMGUI_IMAGE(m_SceneRenderer.GetShadowDebugFrameBuffer()->GetColorAttachment(0)), ImGui::GetContentRegionAvail());
    //     }, { .Opened = &m_ShadowMapDisplayOpened });
    // }

    m_ViewportWindow->OnDraw();
    m_InspectorWindow->OnDraw();
    m_ConsoleWindow->OnDraw();
    m_SceneWindow->OnDraw();
    m_ScriptsInspector->OnDraw();
    m_ContentBrowser->OnDraw();
    m_RendererReport->OnDraw();

    if (m_AssetRegistryViewer->IsVisible())
        m_AssetRegistryViewer->OnDraw();

    for (auto const& asset_window : m_AssetWindows | std::views::values) {
        asset_window->draw(delta);
    }

    std::erase_if(m_AssetWindows, [](auto const& pair) {
        return !pair.second->IsOpen();
    });

    Undo.Commit();

    Project::AssetManager()->CheckForLoadedAssets();
}

void Editor::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<OnKeyPressed>([this](OnKeyPressed const& e) -> bool {
        if (e.Key == Keys::Z && e.Mods.test(KeyMod::Control)) {
            Undo.Undo();
        }

        if (e.Key == Keys::Y && e.Mods.test(KeyMod::Control)) {
            Undo.Redo();
        }

        if (e.Key == Keys::S && e.Mods.test(KeyMod::Control)) {
            Save();
        }

        // TODO: This is really fucky. Figure out a way to properly define what has input focus and what not.
        if (m_SceneWindow->IsFocused() && !m_ViewportWindow->IsFocused() && e.Key == Keys::D && e.Mods.test(KeyMod::Control)) {
            for (auto const& [entity, nothing] : m_SceneWindow->GetSelection()) {
                (void)nothing;
                auto new_handle = m_ActiveScene->CloneEntity(entity);
                if (new_handle != EntityHandle::Invalid) {
                    auto* new_entity = m_ActiveScene->GetEntity(new_handle);
                    new_entity->Name += " (Clone)";
                }
            }
        }
        return false;
    });

    dispatcher.Dispatch<WindowCloseRequest>([this](WindowCloseRequest const&) {
        if (m_ActiveScene != nullptr && m_ActiveScene->IsDirty()) {
            Dialogs::MessageBox data {};
            data.Message = "The current scene has unsaved modifications. Are you sure you want to quit?";
            data.Action = Dialogs::MessageAction::OkCancel;
            switch (Dialogs::ShowMessageBox(data)) {
            case Dialogs::MessageButton::Ok:
            case Dialogs::MessageButton::Yes:
                Application::Self()->Quit();
                break;
            case Dialogs::MessageButton::No:
                break;
            case Dialogs::MessageButton::Cancel:
                break;
            }
        } else {
            Application::Self()->Quit();
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

void Editor::OnDraw(GPU::CommandEncoder& encoder)
{
    auto render_editor_view = [&](Ref<Scene> const& scene) {
        m_SceneRenderer.render(encoder,
            {
                .camera = RenderCamera {
                    .perspective = m_Camera.Perspective(),
                    .view = m_Camera.View(),
                    .rotation = m_Camera.RotationMatrix(),
                    .position = m_Camera.Position,
                    .near = m_Camera.Near,
                    .far = m_Camera.Far,
                    .direction = m_Camera.Direction(),
                },
                .scene = scene.get(),
            },
            false);
    };
    auto render_game_view = [&](Camera const& camera) {
        auto entity = camera.GetOwner();
        m_SceneRenderer.render(encoder,
            {
                .camera = RenderCamera {
                    .perspective = camera.GetPerspective(),
                    .view = inverse(entity->WorldMatrix()),
                    .rotation = entity->Transform.RotationMatrix(),
                    .position = entity->Transform.Position,
                    .near = camera.near,
                    .far = camera.far,
                    .direction = entity->Transform.Forward(),
                },
                .scene = m_PlayScene.get(),
            },
            true);
    };
    switch (m_PlayState) {
    case PlayState::Editing: {
        render_editor_view(m_ActiveScene);
    } break;
    case PlayState::Detached:
    case PlayState::Playing:
    case PlayState::Paused: {
        auto camera = m_PlayScene->FindFirstComponent<Camera>();

        if (camera == nullptr || m_Detached) {
            render_editor_view(m_PlayScene);
        } else {
            render_game_view(*camera.get());
        }
    } break;
    }
}

void Editor::SetPlayState(PlayState new_state)
{
    ZoneScoped;
    switch (m_PlayState) {
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
    m_PlayState = new_state;
}

void Editor::OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc)
{
    m_LogEntries.push_back(LogEntry {
        level,
        std::string(message),
        loc,
    });
}

void Editor::ChangeScene(AssetRef<Scene> scene)
{
    auto LoadScene = [&scene] {
        s_EditorInstance->m_SceneWindow->ClearSelection();

        auto meta = Project::AssetManager()->GetMetadata(scene.GetHandle());

        if (auto scene_json = FileSystem::ReadEntireFile(Project::AssetsFolderPath() / meta.Path)) {
            JsonDeserializer ds(*scene_json);

            auto scene_asset = MakeRef<Scene>();
            scene_asset->Deserialize(ds);
            scene_asset->SetHandle(scene.GetHandle());

            s_EditorInstance->m_ActiveScenePath = meta.Path;
            s_EditorInstance->m_ActiveScene = scene_asset;

            Application::Self()->GetWindow().SetTitle(fmt::format("Fussion - {} - {}", Project::Name(), meta.Name));
        }
    };
    if (s_EditorInstance->m_ActiveScene != nullptr && s_EditorInstance->m_ActiveScene->IsDirty()) {
        Dialogs::MessageBox data {};
        data.Message = "The current scene has unsaved modifications. Are you sure you want to discard them? Selecting 'No' will save the current scene and load the new one.";
        data.Action = Dialogs::MessageAction::YesNoCancel;
        switch (Dialogs::ShowMessageBox(data)) {
        case Dialogs::MessageButton::No: {
            s_EditorInstance->Save();
        }
        case Dialogs::MessageButton::Yes: {
            LoadScene();
        } break;
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
    s_EditorInstance->m_SceneRenderer.resize(new_size);
}
