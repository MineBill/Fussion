#include "EditorLayer.h"

#include <imgui.h>
#include <tracy/Tracy.hpp>

#include "Engin5/Input/Input.h"

void EditorLayer::OnStart()
{
    ZoneScoped;
    m_ViewportWindow = MakePtr<ViewportWindow>(this);
    m_InspectorWindow = MakePtr<InspectorWindow>(this);
    m_ConsoleWindow = MakePtr<ConsoleWindow>(this);
    m_SceneWindow = MakePtr<SceneWindow>(this);

    ImGui::LoadIniSettingsFromDisk("Assets/EditorLayout.ini");
}

void EditorLayer::OnEnable()
{
}

void EditorLayer::OnDisable()
{
}

void EditorLayer::OnUpdate(const f32 delta)
{
    (void)delta;
    ZoneScoped;

    if (Engin5::Input::IsKeyPressed(Engin5::KeyboardKey::G)) {
        LOG_DEBUGF("Delta: {}", delta);
    }
    ImGui::DockSpaceOverViewport();

    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open..")) {

        }
        ImGui::EndMenu();
    }

    static bool show_demo_window = false;
    if (ImGui::BeginMenu("Extra")) {
        ImGui::Checkbox("Demo Window", &show_demo_window);
        if (ImGui::MenuItem("Save Layout")) {
            ImGui::SaveIniSettingsToDisk("Assets/EditorLayout.ini");
        }

        ImGui::EndMenu();
    }

    ImGui::Text("Delta: %f", delta);
    ImGui::EndMainMenuBar();

    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    m_ViewportWindow->OnDraw();
    m_InspectorWindow->OnDraw();
    m_ConsoleWindow->OnDraw();
    m_SceneWindow->OnDraw();
}

void EditorLayer::OnEvent(Engin5::Event& event)
{
}

void EditorLayer::OnDraw(Ref<Engin5::CommandBuffer> cmd)
{
}
