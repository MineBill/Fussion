#include "Editor.h"

#include "Engin5/Input/Input.h"
#include "Engin5/Core/Application.h"
#include "Engin5/Scene/Components/BaseComponents.h"
#include "Engin5/Scene/Scene.h"

#include <imgui.h>
#include <tracy/Tracy.hpp>

#include "Engin5/Scripting/ScriptingEngine.h"

Editor* Editor::s_EditorInstance = nullptr;
using namespace Engin5;

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

    ImGui::LoadIniSettingsFromDisk("Assets/EditorLayout.ini");

    m_Camera.Resize(Application::Instance()->GetWindow().GetSize());
    m_SceneRenderer.Init();

    OnViewportResized(Vector2(300, 300));

    m_ActiveScene = MakeRef<Scene>();
    auto const entity = m_ActiveScene->CreateEntity();
    m_ActiveScene->CreateEntity("Bob");
    m_ActiveScene->CreateEntity("Mike");
    auto const e = m_ActiveScene->CreateEntity("John");
    e->AddComponent<PointLight>();

    auto stream = ScriptingEngine::Get().DumpCurrentTypes();
    std::ofstream file("Assets/Scripts/as.predefined");
    file << stream.str();
    file.close();

    auto assembly = ScriptingEngine::Get().CompileAssembly(std::filesystem::current_path().parent_path() / "Editor" / "Assets" / "Scripts", "Game");
    if (auto klass = assembly->GetClass("Test"); klass.has_value()) {
        if (ScriptInstance script = klass->CreateInstance(); script.IsValid()) {
            script.CallMethod("OnStart");
        }
    }

    m_ViewportWindow->OnStart();
    m_InspectorWindow->OnStart();
    m_ConsoleWindow->OnStart();
    m_SceneWindow->OnStart();
    m_ScriptsInspector->OnStart();
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
        LOG_DEBUGF("Delta: {}", delta);
    }
    ImGui::DockSpaceOverViewport();

    static bool show_demo_window = false;
    ImGui::BeginMainMenuBar();
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open..")) {

            }
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
