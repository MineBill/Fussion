#include "Editor.h"

#include "Fussion/Input/Input.h"
#include "Fussion/Core/Application.h"
#include "Fussion/Scene/Components/BaseComponents.h"

#include "Fussion/OS/Dialog.h"
#include "Fussion/Scripting/ScriptingEngine.h"

#include <imgui.h>
#include "ImGuizmo.h"
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "EditorApplication.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Events/KeyboardEvents.h"

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
    m_Style.Init();

    m_Camera.Resize(Application::Instance()->GetWindow().GetSize());
    m_Camera.Position = Vector3(0, 2, 0);
    m_SceneRenderer.Init();

    OnViewportResized(Vector2(300, 300));

    m_ViewportWindow->OnStart();
    m_InspectorWindow->OnStart();
    m_ConsoleWindow->OnStart();
    m_SceneWindow->OnStart();
    m_ContentBrowser->OnStart();

    m_ScriptsInspector->OnStart();
    m_ScriptsInspector->Hide();
}

void Editor::OnEnable() {}

void Editor::OnDisable() {}

void Editor::OnUpdate(const f32 delta)
{
    ZoneScoped;

    if (auto scene = m_ActiveScene.Get()) {
        scene->OnUpdate(delta);
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
                    m_ActiveScene = Project::ActiveProject()->GetAssetManager()->CreateAsset<Scene>("TestScene.fsn");
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

            ImGui::Separator();

            if (ImGui::MenuItem("Quit")) {
                Quit();
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

    Undo.Commit();
}

void Editor::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<OnKeyPressed>([this](OnKeyPressed const& e) -> bool {
        if (e.Key == KeyboardKey::Z && e.Mods.Test(KeyMod::Control)) {
            LOG_DEBUG("FUCK SHIT UNDO");
            Undo.Undo();
        }
        if (e.Key == KeyboardKey::Y && e.Mods.Test(KeyMod::Control)) {
            LOG_DEBUG("FUCK SHIT REDO");
            Undo.Redo();
        }
        return false;
    });

    m_Camera.HandleEvent(event);

    m_ViewportWindow->OnEvent(event);
    m_InspectorWindow->OnEvent(event);
    m_ConsoleWindow->OnEvent(event);
    m_SceneWindow->OnEvent(event);
    m_ScriptsInspector->OnEvent(event);
    m_ContentBrowser->OnEvent(event);
}

void Editor::OnDraw(Ref<CommandBuffer> cmd)
{
    m_SceneRenderer.Render(cmd, {
        .Camera = RenderCamera{
            .Perspective = m_Camera.GetPerspective(),
            .View = m_Camera.GetView(),
            .Position = m_Camera.Position,
        },
        .Scene = m_ActiveScene.Get(),
    });
}

void Editor::Quit()
{
    LOG_DEBUG("Quitting application");
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
    s_EditorInstance->m_ActiveScene = scene;
}

void Editor::OnViewportResized(Vector2 new_size)
{
    ZoneScoped;

    s_EditorInstance->m_Camera.Resize(new_size);
    s_EditorInstance->m_SceneRenderer.Resize(new_size);
}
