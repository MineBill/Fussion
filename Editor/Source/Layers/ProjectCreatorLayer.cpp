#include "EditorPCH.h"
#include "ProjectCreatorLayer.h"

#include "EditorApplication.h"
#include "EditorUI.h"
#include "ImGuiHelpers.h"
#include "Fussion/Core/Application.h"
#include "Fussion/OS/Dialog.h"
#include "Fussion/Util/TextureImporter.h"

#include <imgui_internal.h>

using namespace Fussion;

void ProjectCreatorLayer::OnStart()
{
    LOG_INFOF("TestLayer::OnStart");
    ImGui::ClearIniSettings();
    ImGui::ClearWindowSettings("Window");

    Application::Instance()->GetWindow().SetTitle("Fussion - Project Creator");

    LOG_INFOF("{}", std::filesystem::current_path());
    m_TestTexture = TextureImporter::LoadTextureFromFile("Assets/coords.png").Value();
}

void ProjectCreatorLayer::OnUpdate(f32 delta)
{
    (void)delta;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    EUI::Window("Window", [&] {
        ImGui::PushFont(EditorStyle::GetStyle().Fonts[EditorFont::RegularHuge]);
        ImGuiH::Text("Project Creator");
        ImGui::PopFont();

        ImGui::Separator();

        constexpr auto width = 150;
        auto avail_width = ImGui::GetContentRegionAvail().x;
        ImGui::BeginChild("Buttons", Vector2{ width, 0 }, 0, ImGuiWindowFlags_NoSavedSettings);

        constexpr auto padding = 20;
        constexpr auto button_width = width - padding;
        ImGui::SetCursorPosX(padding / 2);
        ImGui::SetCursorPosY(padding / 2);
        EUI::Button("New", [&] {
            m_OpenNewProjectPopup = true;
        }, { .Style = ButtonStyleProjectCreator, .Size = Vector2{ button_width, 0 } });
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Content", Vector2{ avail_width - width - ImGui::GetStyle().FramePadding.x * 2, 0 });
        if (ImGui::BeginTabBar("MyTabBar")) {

            if (ImGui::BeginTabItem("Projects")) {
                ImGui::Image(m_TestTexture->GetImage().View, ImGui::GetContentRegionAvail());
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        if (m_OpenNewProjectPopup) {
            ImGui::OpenPopup("New Project");
            m_OpenNewProjectPopup = false;
        }

        auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        auto size = Vector2(viewport->Size) / 2.0f;
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowPos(Vector2(viewport->WorkPos) + size / 2.0f);
        EUI::ModalWindow("New Project", [&] {
            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(5, 5));
            ImGui::BeginChild("awd", { 0, size.Y - 75 });
            static std::string project_name;
            if (EUI::Property("Project Name", &project_name)) {
                m_ProjectNameValidated = !project_name.empty() && !StringUtils::IsWhitespace(project_name);
            }

            static std::string project_folder;

            EUI::Property("Project Folder", [] {
                EUI::Button("Select", [] {
                    project_folder = Dialogs::ShowDirectoryPicker().string();
                }, { .Style = ButtonStyleViewportButton });

                ImGui::SameLine();

                ImGui::InputText("", &project_folder);
            });

            ImGui::Spacing();

            if (!m_ProjectNameValidated) {
                ImGui::PushFont(EditorStyle::GetStyle().Fonts[EditorFont::Bold]);
                defer(ImGui::PopFont());
                ImGui::Text("Invalid project name");
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();

            ImGui::Separator();

            EUI::Button("Cancel", [] {
                ImGui::CloseCurrentPopup();
            }, { .Style = ButtonStyleProjectCreator });
            ImGui::SameLine();

            if (!m_ProjectNameValidated)
                ImGui::BeginDisabled();

            EUI::Button("Create", [] {
                auto path = EditorApplication::CreateProject(fs::path(project_folder));
                EditorApplication::CreateEditorFromProjectCreator(path);
                ImGui::CloseCurrentPopup();
            }, { .Style = ButtonStyleProjectCreator });

            if (!m_ProjectNameValidated)
                ImGui::EndDisabled();
        }, { .Flags = flags });
    }, { .Style = WindowStyleCreator, .Flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings, .Size = {}, .UseChild = false });
}

void ProjectCreatorLayer::OnEvent(Event& event)
{
    if (event.Type() == EventType::WindowClose) {
        Application::Instance()->Quit();
    }
}
