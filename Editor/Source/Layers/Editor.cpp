#include "Editor.h"
#include "EditorUI.h"
#include "Fussion/Util/TextureImporter.h"
#include "Serialization/SceneSerializer.h"

#include <Fussion/Input/Input.h>
#include <Fussion/Core/Application.h>
#include <Fussion/Scene/Components/BaseComponents.h>
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Events/ApplicationEvents.h>
#include <Fussion/Events/KeyboardEvents.h>
#include <Fussion/OS/FileSystem.h>
#include <Fussion/Scripting/ScriptingEngine.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <magic_enum/magic_enum.hpp>
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

void Editor::OnStart()
{
    ZoneScoped;
    m_ViewportWindow = MakePtr<ViewportWindow>(this);
    m_InspectorWindow = MakePtr<InspectorWindow>(this);
    m_ConsoleWindow = MakePtr<ConsoleWindow>(this);
    m_SceneWindow = MakePtr<SceneTreeWindow>(this);
    m_ScriptsInspector = MakePtr<ScriptsInspector>(this);
    m_ContentBrowser = MakePtr<ContentBrowser>(this);

    ScriptingEngine::Get().CompileGameAssembly(Project::ActiveProject()->GetScriptsFolder());
    FileSystem::WriteEntireFile(Project::ActiveProject()->GetScriptsFolder() / "as.predefined", ScriptingEngine::Get().DumpCurrentTypes().str());

    m_Watcher = FileWatcher::Create(Project::ActiveProject()->GetScriptsFolder());
    m_Watcher->RegisterListener([this](std::filesystem::path const& path, FileWatcher::EventType type) {
        using namespace std::chrono_literals;
        // Wait a bit for the file lock to be released.
        std::this_thread::sleep_for(100ms);
        ScriptingEngine::Get().CompileGameAssembly(Project::ActiveProject()->GetScriptsFolder());
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

    m_ScriptsInspector->OnStart();
    m_ScriptsInspector->Hide();

    OnBeginPlay += [this] {
        LOG_DEBUG("On Begin Play");
        auto serializer = MakePtr<SceneSerializer>();
        auto meta = Project::ActiveProject()->GetAssetManager()->GetMetadata(m_ActiveScene->GetHandle());
        m_PlayScene = serializer->Load(meta)->As<Scene>();

        if (m_PlayScene)
            m_PlayScene->OnStart();
    };

    OnStopPlay += [this] {
        LOG_DEBUG("On Stop Play");
        // m_SceneWindow->ClearSelection();
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

void Editor::Save()
{
    Project::ActiveProject()->Save();

    if (m_State == PlayState::Editing && m_ActiveScene != nullptr) {
        auto serializer = MakePtr<SceneSerializer>();
        EditorAssetMetadata meta{};
        meta.Path = m_ActiveScenePath;
        serializer->Save(meta, m_ActiveScene);
        m_ActiveScene->SetDirty(false);
    }
}

void Editor::OnUpdate(f32 delta)
{
    ZoneScoped;

    GenericAssetPicker.Update();

    switch (m_State) {
    case PlayState::Editing: {
        if (m_ActiveScene) {
            m_ActiveScene->Tick();
            m_ActiveScene->OnDebugDraw(DebugDrawContext);
        }
    }
    break;
    case PlayState::Playing: {
        if (m_PlayScene) {
            m_PlayScene->OnUpdate(delta);
            m_PlayScene->OnDebugDraw(DebugDrawContext);
        }
    }
    break;
    case PlayState::Paused: {}
    break;
    }

    m_Camera.SetFocus(m_ViewportWindow->IsFocused());
    m_Camera.OnUpdate(delta);

    ImGui::DockSpaceOverViewport();

    static bool show_demo_window = false;
    ImGui::BeginMainMenuBar();
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("New..")) {
                if (ImGui::MenuItem("Create Scene")) {
                    ChangeScene(Project::ActiveProject()->GetAssetManager()->CreateAsset<Scene>("TestScene.fsn"));
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
        ImGuiDockNodeFlags_NoDockingOverOther;

    ImGuiWindowClass klass;
    klass.DockNodeFlagsOverrideSet = flags;

    ImGui::SetNextWindowClass(&klass);
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

        ImGui::SetItemTooltip("Pause");

        auto min_step_button = ImGui::GetItemRectMin();
        auto max = ImGui::GetItemRectMax();

        list->ChannelsSetCurrent(0);

        Color color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        color = color.Lighten(0.2f);

        // auto middle = (max.x - min_step_button.x) / 2.0f;
        list->AddRectFilled(min, max, color.ToABGR(), 3);

        // imgui.DrawList_AddLine(list, min_step_button + vec2{middle, 0}, max - vec2{middle, 0}, color_to_abgr(line_color))
        // list->AddLine()
        list->ChannelsMerge();
    }, { .Flags = ImGuiWindowFlags_NoDecoration });

    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    m_ViewportWindow->OnDraw();
    m_InspectorWindow->OnDraw();
    m_ConsoleWindow->OnDraw();
    m_SceneWindow->OnDraw();
    m_ScriptsInspector->OnDraw();
    m_ContentBrowser->OnDraw();

    for (auto const& asset_window : m_AssetWindows | std::views::values) {
        asset_window->OnDraw(delta);
    }

    std::erase_if(m_AssetWindows, [](auto const& pair) {
        return !pair.second->IsOpen();
    });

    Undo.Commit();
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
                Application::Instance()->Quit();
                break;
            case Dialogs::MessageButton::Yes:
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
    m_SceneRenderer.Render(cmd, {
        .Camera = RenderCamera{
            .Perspective = m_Camera.GetPerspective(),
            .View = m_Camera.GetView(),
            .Position = m_Camera.Position,
            .Near = m_Camera.Near,
            .Far = m_Camera.Far,
            .Direction = m_Camera.GetDirection(),
        },
        .Scene = m_State == PlayState::Editing ? m_ActiveScene.get() : m_PlayScene.get(),
    });
}

void Editor::SetPlayState(PlayState new_state)
{
    ZoneScoped;
    switch (m_State) {
    case PlayState::Editing:
        switch (new_state) {
        case PlayState::Editing:
            // noop
            return;
        case PlayState::Playing:
            OnBeginPlay.Fire();
            break;
        case PlayState::Paused:
            // noop
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
        case PlayState::Paused:
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

        auto serializer = MakePtr<SceneSerializer>();
        auto meta = Project::ActiveProject()->GetAssetManager()->GetMetadata(scene.Handle());
        s_EditorInstance->m_ActiveScene = serializer->Load(meta)->As<Scene>();
        s_EditorInstance->m_ActiveScene->SetHandle(scene.Handle());
        s_EditorInstance->m_ActiveScenePath = meta.Path;
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
            break;
        }
    } else {
        LoadScene();
    }
}

void Editor::OnViewportResized(Vector2 new_size)
{
    ZoneScoped;

    s_EditorInstance->m_Camera.Resize(new_size);
    s_EditorInstance->m_SceneRenderer.Resize(new_size);
}
