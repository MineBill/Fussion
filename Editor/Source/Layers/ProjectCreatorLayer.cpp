#include "ProjectCreatorLayer.h"

#include "EditorUI.h"
#include "ImGuiHelpers.h"
#include "Fussion/Core/Application.h"
#include "Fussion/OS/Dialog.h"

#include <imgui_internal.h>

void ProjectCreatorLayer::OnStart()
{
    LOG_INFOF("TestLayer::OnStart");
    ImGui::ClearIniSettings();
    ImGui::ClearWindowSettings("Window");
}

void ProjectCreatorLayer::OnUpdate(f32 delta)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    // if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
    //     if (!m_StartedDragging) {
    //         m_MouseDragStarPos = viewport->Pos;
    //         m_Offset = Vector2(ImGui::GetMousePos()) - viewport->Pos;
    //         m_StartedDragging = true;
    //     }
    //
    //     Fussion::Application::Instance()->GetWindow().SetPosition(Vector2(ImGui::GetMousePos()) - m_Offset);
    // } else {
    //     m_StartedDragging = false;
    // }

    EUI::Window("Window", [&] {
        ImGui::PushFont(EditorStyle::GetStyle().Fonts[EditorFont::RegularHuge]);
        ImGuiH::Text("Project Creator");
        ImGui::PopFont();

        ImGui::Separator();

        constexpr auto width = 150;
        auto avail_width = ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("Buttons", Vector2{ width, 0 }, 0, ImGuiWindowFlags_NoSavedSettings);
        EUI::Button("New", [&] {
            m_OpenNewProjectPopup = true;
        }, ButtonStyleProjectCreator, Vector2{ 125, 0 }, 0.5);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Content", Vector2{ avail_width - width - ImGui::GetStyle().FramePadding.x * 2, 0 });
        if (ImGui::BeginTabBar("MyTabBar")) {

            if (ImGui::BeginTabItem("Projects")) {
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        if (m_OpenNewProjectPopup) {
            ImGui::OpenPopup("NewProjectWindow");
            m_OpenNewProjectPopup = false;
        }

        auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        auto size = Vector2(viewport->Size) / 2.0f;
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowPos(Vector2(viewport->WorkPos) + size / 2.0f);
        if (ImGui::BeginPopupModal("NewProjectWindow", nullptr, flags)) {
            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(5, 5));
            ImGui::BeginChild("awd", { 0, size.Y - 75 });
            static std::string project_name;
            EUI::Property("Project Name", &project_name);

            static std::string project_folder;

            EUI::Property("Project Folder", [] {
                EUI::Button("Select", [] {
                    project_folder = Fussion::Dialogs::ShowDirectoryPicker().string();
                }, ButtonStyleProjectCreatorSmall);

                ImGui::SameLine();

                ImGui::InputText("", &project_folder);
            });

            ImGui::EndChild();
            ImGui::PopStyleVar();

            ImGui::Separator();

            EUI::Button("Cancel", [] {}, ButtonStyleProjectCreator);
            ImGui::SameLine();
            EUI::Button("Create", [] {}, ButtonStyleProjectCreator, { 0, 0 });
            ImGui::EndPopup();
        }
    }, { .Style = WindowStyleCreator, .Flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings, .Size = {} });

    // ImGui::ShowDemoWindow();
}
