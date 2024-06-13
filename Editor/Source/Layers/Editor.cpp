#include "Editor.h"

#include "Fussion/Input/Input.h"
#include "Fussion/Core/Application.h"
#include "Fussion/Scene/Components/BaseComponents.h"

#include "Fussion/OS/Dialog.h"
#include "Fussion/Scripting/ScriptingEngine.h"

#include <imgui.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "Fussion/Assets/AssetRef.h"

Editor* Editor::s_EditorInstance = nullptr;
using namespace Fussion;


Editor::Editor()
{
    if (s_EditorInstance != nullptr) {
        PANIC("EditorLayer already exists!");
    }
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

    ImGui::LoadIniSettingsFromDisk("Assets/EditorLayout.ini");

    m_Camera.Resize(Application::Instance()->GetWindow().GetSize());
    m_SceneRenderer.Init();

    OnViewportResized(Vector2(300, 300));


    auto stream = ScriptingEngine::Get().DumpCurrentTypes();
    std::ofstream file("Assets/Scripts/as.predefined");
    file << stream.str();
    file.close();

    m_ViewportWindow->OnStart();
    m_InspectorWindow->OnStart();
    m_ConsoleWindow->OnStart();
    m_SceneWindow->OnStart();
    m_ScriptsInspector->OnStart();
    m_ContentBrowser->OnStart();
}

void Editor::OnEnable()
{
}

void Editor::OnDisable()
{
}

void Editor::OnUpdate(const f32 delta)
{
    ZoneScoped;

    m_Camera.SetFocus(m_ViewportWindow->IsFocused());
    m_Camera.OnUpdate(delta);

    if (Input::IsKeyPressed(KeyboardKey::G)) {
        using namespace Dialogs;

        // MessageBox box {
        //     .Title = "Hello",
        //     .Message = "Are you stupid?",
        //     .Type = MessageType::Info,
        //     .Action = MessageAction::YesNo
        // };
        // LOG_DEBUGF("Got: {}", magic_enum::enum_name(ShowMessageBox(box)));

        // auto path = ShowFilePicker("Project", {"*.*"});
        // LOG_DEBUGF("{}", path.extension().string());
    }
    ImGui::DockSpaceOverViewport();

    static bool show_demo_window = false;
    ImGui::BeginMainMenuBar();
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::BeginMenu("New..")) {
                if (ImGui::MenuItem("Create Scene")) {
                    m_ActiveScene = Project::ActiveProject()->GetAssetManager()->CreateAsset<Scene>("Pepegas.scene");
                    // m_ActiveScene = AssetManager::GetAsset<Scene>(handle);
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (!m_ActiveScene)
                ImGui::BeginDisabled();

            if (ImGui::MenuItem("Save..", "Ctrl+S")) {
                Project::ActiveProject()->Save();
                Project::ActiveProject()->GetAssetManager()->SaveAsset(m_ActiveScene.Handle());
            }

            if (!m_ActiveScene)
                ImGui::EndDisabled();

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

        ImGui::Text("Delta: %f", delta);
    }
    ImGui::EndMainMenuBar();

    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    m_ViewportWindow->OnDraw();
    m_InspectorWindow->OnDraw();
    m_ConsoleWindow->OnDraw();
    m_SceneWindow->OnDraw();
    m_ScriptsInspector->OnDraw();
    m_ContentBrowser->OnDraw();
}

void Editor::OnEvent(Event& event)
{
    m_Camera.HandleEvent(event);
}

void Editor::OnDraw(Ref<CommandBuffer> cmd)
{
    m_SceneRenderer.Render(cmd, {
        .Camera = RenderCamera {
            .Perspective = m_Camera.GetPerspective(),
            .View = m_Camera.GetView(),
            .Position = m_Camera.GetPosition(),
        }
    });
}

void Editor::OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc)
{
    m_LogEntries.push_back(LogEntry {
        level,
        std::string(message),
        loc,
    });
}

void Editor::OnViewportResized(Vector2 new_size)
{
    ZoneScoped;

    s_EditorInstance->m_Camera.Resize(new_size);
    s_EditorInstance->m_SceneRenderer.Resize(new_size);
}
