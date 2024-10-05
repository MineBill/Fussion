#include "EditorPCH.h"
#include "ProjectCreatorLayer.h"

#include "EditorApplication.h"
#include "EditorUI.h"
#include "Fussion/Core/Application.h"
#include "Fussion/OS/Dialog.h"
#include "Fussion/OS/FileSystem.h"
#include "Fussion/OS/System.h"
#include "Fussion/Serialization/JsonSerializer.h"
#include "ImGuiHelpers.h"

#include <imgui_internal.h>

using namespace Fussion;

void ProjectCreatorLayer::OnStart()
{
    LOG_INFOF("TestLayer::OnStart");
    ImGui::ClearIniSettings();
    ImGui::ClearWindowSettings("Window");

    Application::Self()->GetWindow().SetTitle("Fussion - Project Creator");

    LoadProjects();
}

void ProjectCreatorLayer::OnUpdate(f32 delta)
{
    (void)delta;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    struct {
        bool Requested { false };
        std::filesystem::path Path {};

        void Set(std::filesystem::path const& path)
        {
            Path = path;
            Requested = true;
        }
    } request;

    EUI::Window("Window", [&] {
        ImGui::PushFont(EditorStyle::Style().Fonts[EditorFont::RegularHuge]);
        ImGuiH::Text("Project Creator");
        ImGui::PopFont();

        ImGui::Separator();

        constexpr auto width = 150;
        auto avail_width = ImGui::GetContentRegionAvail().x;

#pragma region Side Panel Buttons
        ImGui::BeginChild("Buttons", Vector2 { width, 0 }, 0, ImGuiWindowFlags_NoSavedSettings);

        constexpr auto padding = 20.f;
        constexpr auto button_width = width - padding;
        ImGui::SetCursorPosX(padding / 2.f);
        ImGui::SetCursorPosY(padding / 2.f);
        EUI::Button("New", [&] {
            m_OpenNewProjectPopup = true;
        },
            { .style = ButtonStyleProjectCreator, .size = Vector2 { button_width, 0 } });

        ImGui::SetCursorPosX(padding / 2.f);
        EUI::Button("Import", [&] {
            m_OpenImportProjectPopup = true;
        },
            { .style = ButtonStyleProjectCreator, .size = Vector2 { button_width, 0 } });
        ImGui::EndChild();
#pragma endregion

        ImGui::SameLine();

        ImGui::BeginChild("Content", Vector2 { avail_width - width - ImGui::GetStyle().FramePadding.x * 2, 0 }, ImGuiChildFlags_Border);
        if (ImGui::BeginTabBar("MyTabBar")) {

            if (ImGui::BeginTabItem("Projects")) {
                ImGui::BeginChild("scrolling_region");

                for (auto const& project : m_Projects) {
                    auto list = ImGui::GetWindowDrawList();
                    list->ChannelsSplit(2);
                    list->ChannelsSetCurrent(1);
#define PADDED(x, y) ImGui::SetCursorPos(ImGui::GetCursorPos() + Vector2(x, y));

                    auto start = ImGui::GetCursorScreenPos();

                    ImGui::BeginGroup();
                    PADDED(5, 5);

                    EUI::WithEditorFont(EditorFont::RegularBig, [&] {
                        ImGui::TextUnformatted(project.Name.c_str());
                    });

                    PADDED(5, 5);
                    EUI::WithEditorFont(EditorFont::RegularSmall, [&] {
                        ImGui::PushStyleColor(ImGuiCol_Text, Color::Gray);
                        defer(ImGui::PopStyleColor());

                        ImGui::TextUnformatted(project.Location.string().c_str());
                    });

                    ImGui::EndGroup();

                    auto end = ImGui::GetCursorScreenPos();
                    end.x += ImGui::GetContentRegionAvail().x - 5.f;

                    list->ChannelsSetCurrent(0);

                    ImGui::SetCursorScreenPos(start);
                    ImGui::Dummy(end - start);
                    if (ImGui::IsItemHovered() || ImGui::IsItemFocused()) {
                        list->AddRectFilled(start, end, 0xFF383838, 5);
                    }
                    if (ImGui::IsItemClicked()) {
                        request.Set(project.Location);
                    }
                    list->ChannelsMerge();

                    ImGui::Separator();
                    ImGui::Spacing();
                }
                ImGui::EndChild();

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        if (m_OpenNewProjectPopup) {
            ImGui::OpenPopup("New Project");
            m_OpenNewProjectPopup = false;
        }

        if (m_OpenImportProjectPopup) {
            ImGui::OpenPopup("Import Project");
            m_OpenImportProjectPopup = false;
        }

        auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        auto size = Vector2(viewport->Size) / 2.0f;
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowPos(Vector2(viewport->WorkPos) + size / 2.0f);
        EUI::ModalWindow("New Project", [&] {
            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(5, 5));
            ImGui::BeginChild("awd", { 0, size.y - 75 });
            static std::string project_name;
            if (EUI::Property("Project Name", &project_name)) {
                m_ProjectNameValidated = !project_name.empty() && !StringUtils::IsWhitespace(project_name);
            }

            static std::string project_folder;

            EUI::Property("Project Folder", [] {
                EUI::Button("Select", [] {
                    project_folder = Dialogs::ShowDirectoryPicker().string();
                },
                    { .style = ButtonStyleViewportButton });

                ImGui::SameLine();

                ImGui::InputText("", &project_folder);
            });

            ImGui::Spacing();

            if (!m_ProjectNameValidated || project_folder.empty()) {
                ImGui::PushFont(EditorStyle::Style().Fonts[EditorFont::Bold]);
                defer(ImGui::PopFont());
                ImGui::Text("Invalid project name and/or folder.");
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();

            ImGui::Separator();

            EUI::Button("Cancel", [] {
                ImGui::CloseCurrentPopup();
            },
                { .style = ButtonStyleProjectCreator });
            ImGui::SameLine();

            if (!m_ProjectNameValidated)
                ImGui::BeginDisabled();

            EUI::Button("Create", [&] {
                auto path = EditorApplication::CreateProject(fs::path(project_folder), project_name);
                AddProject(path);

                request.Set(path);
                ImGui::CloseCurrentPopup();
            },
                { .style = ButtonStyleProjectCreator });

            if (!m_ProjectNameValidated)
                ImGui::EndDisabled();
        },
            { .flags = flags });

        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowPos(Vector2(viewport->WorkPos) + size / 2.0f);
        EUI::ModalWindow("Import Project", [&] {
            static std::string project_path;
            EUI::Property("Project Folder", [] {
                EUI::Button("Select", [] {
                    auto file = Dialogs::ShowFilePicker(Dialogs::FilePickerFilter {
                        .name = "Project File",
                        .file_patterns = { "*.fsnproj" } });

                    project_path = file[0].string();
                },
                    { .style = ButtonStyleViewportButton });
                ImGui::SameLine();
                ImGui::InputText("", &project_path);
            });

            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Import")) {
                AddProject(project_path);
                ImGui::CloseCurrentPopup();
            }
        },
            { .flags = flags });
    },
        { .style = WindowStyleCreator, .flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings, .size = {}, .use_child = false });

    if (request.Requested) {
        EditorApplication::CreateEditorFromProjectCreator(request.Path);
    }
}

void ProjectCreatorLayer::OnEvent(Event& event)
{
    if (event.GetType() == EventType::WindowClose) {
        Application::Self()->Quit();
    }
}

void ProjectCreatorLayer::Serialize(Serializer& s) const
{
    s.BeginArray("projects", m_Projects.size());
    for (auto const& project : m_Projects) {
        s.BeginObject("", 2);
        s.Write("name", project.Name);
        s.Write("location", project.Location);
        s.EndObject();
    }
    s.EndArray();
}

void ProjectCreatorLayer::Deserialize(Deserializer& ds)
{
    size_t size;
    ds.BeginArray("projects", size);
    m_Projects.reserve(size);

    for (usz i = 0; i < size; ++i) {
        auto& project = m_Projects.emplace_back();
        usz obj_size;
        ds.BeginObject("", obj_size);
        ds.Read("name", project.Name);
        ds.Read("location", project.Location);
        ds.EndObject();
    }

    ds.EndArray();
}

void ProjectCreatorLayer::AddProject(std::filesystem::path const& path)
{
    if (!exists(path) || is_directory(path)) {
        LOG_WARNF("Invalid project file");
        return;
    }

    auto project_file = FileSystem::ReadEntireFile(path);
    auto j = json::parse(*project_file);

    if (!j.contains("Name")) {
        LOG_WARN("Project file doesn't contain a name field");
        return;
    }
    if (!j["Name"].is_string()) {
        LOG_WARN("Name field is not a string");
        return;
    }

    auto name = j["Name"].get<std::string>();

    m_Projects.emplace_back(name, path);

    SaveProjects();
}

void ProjectCreatorLayer::SaveProjects() const
{
    auto projects_location = GetKnownFolder(System::KnownFolders::AppData) / "Fussion" / "ProjectCreator" / "Projects.json";

    JsonSerializer js;
    js.Initialize();
    Serialize(js);

    FileSystem::WriteEntireFile(projects_location, js.ToString());
}

void ProjectCreatorLayer::LoadProjects()
{
    auto projects_location = GetKnownFolder(System::KnownFolders::AppData) / "Fussion" / "ProjectCreator" / "Projects.json";

    if (auto file = FileSystem::ReadEntireFile(projects_location)) {
        JsonDeserializer ds(*file);
        ds.Initialize();

        Deserialize(ds);
    }
}
