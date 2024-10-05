#include "EditorPCH.h"
#include "ContentBrowser.h"
#include "AssetWindows/Texture2DWindow.h"
#include "EditorUI.h"
#include "ImGuiHelpers.h"
#include "Layers/Editor.h"
#include "Layers/ImGuiLayer.h"
#include "Project/Project.h"
#include "Serialization/AssetSerializer.h"

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/OS/Dialog.h"
#include <Fussion/Util/TextureImporter.h>

#include "Fussion/Input/Input.h"
#include "Fussion/OS/System.h"
#include "imgui.h"

#include <ranges>

using namespace Fussion;
namespace fs = std::filesystem;

void ContentBrowserWindow::NamePopup::Show(std::function<void(std::string)> const& callback)
{
    m_Callback = callback;
    m_Show = true;
    m_Opened = true;
}

void ContentBrowserWindow::NamePopup::Update()
{
    if (m_Show) {
        ImGui::OpenPopup("NamePicker");
        m_Show = false;
    }

    bool was_opened = m_Opened;
    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    EUI::ModalWindow("NamePicker", [&] {
        ImGuiH::Text("Please pick a name:");
        ImGui::Separator();

        // Set the keyboard focus here once, when the window first appears.
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
            ImGui::SetKeyboardFocusHere(0);

        if (ImGui::InputText("##input", &m_Name)) {
            m_ShowError = Project::AssetManager()->IsPathAnAsset(m_Name, false);
        }

        if (m_ShowError) {
            ImGui::Image(EditorStyle::Style().EditorIcons[EditorIcon::Error]->GetTexture().View, Vector2(16, 16));
            ImGui::SameLine();
            ImGui::TextUnformatted("Item already exists");
        }

        EUI::Button("Accept", [this] {
            Accept();
        },
            { .disabled = m_ShowError });
        ImGui::SameLine();
        EUI::Button("Cancel", [&] {
            m_Opened = false;

            // Prevent m_Callback from being called.
            was_opened = false;
        });

        if (Input::IsKeyPressed(Keys::Enter)) {
            Accept();
        }
    },
        { .flags = flags, .opened = &m_Opened });

    if (was_opened && !m_Opened) {
        m_Callback(m_Name);
        m_Callback = nullptr;
        m_Name.clear();
    }
}

void ContentBrowserWindow::NamePopup::Accept()
{
    if (!m_ShowError) {
        m_Opened = false;
    }
}

void ContentBrowserWindow::OnStart()
{
    m_Root = Project::AssetsFolderPath();
    ChangeDirectory(m_Root);

    m_FileTypes[".png"] = AssetType::Texture2D;
    m_FileTypes[".jpg"] = AssetType::Texture2D;
    m_FileTypes[".jpeg"] = AssetType::Texture2D;
    m_FileTypes[".glb"] = AssetType::Model;
    m_FileTypes[".gltf"] = AssetType::Model;
    m_FileTypes[".hdr"] = AssetType::Texture2D;

    m_ImportFilter.name = "Supported Asset Files";
    for (auto const& file_type : m_FileTypes | std::views::keys) {
        m_ImportFilter.file_patterns.push_back(std::format("*{}", file_type));
    }
}

void ContentBrowserWindow::OnDraw()
{
    EUI::Window("Content", [&] {
        m_IsFocused = ImGui::IsWindowFocused();
        m_NamePopup.Update();

        EUI::Button("Import", [&] {
            for (auto const& path : Fussion::Dialogs::ShowFilePicker(m_ImportFilter, true)) {
                ImportFile(path);
            }
            RefreshContents();
        });
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        ImGuiH::Text("Path: {}", m_RelativeToRootStringPath);

        ImGui::SameLine();
        auto width = ImGui::GetContentRegionAvail().x - (16 * 2 + ImGui::GetStyle().FramePadding.x);
        ImGui::Dummy({ width, 0 });

        ImGui::SameLine();

        auto& style = EditorStyle::Style();
        EUI::ImageButton(style.EditorIcons[EditorIcon::Dots], [&] {
            ImGui::OpenPopup("ContentBrowserOptions");
        },
            { .size = Vector2 { 16, 16 } });
        if (ImGui::BeginItemTooltip()) {
            ImGui::TextUnformatted("Press to view content browser options.");
            ImGui::EndTooltip();
        }

        EUI::Popup("ContentBrowserOptions", [&] {
            EUI::Property("Padding", &m_Padding, EUI::PropTypeRange { .min = 2, .max = 32 });
            EUI::Property("Thumbnail Size", &m_ThumbnailSize, EUI::PropTypeRange { .min = 16, .max = 128 });
        });

        ImGui::Separator();

        ImGui::BeginChild("##child");

        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::BeginMenu("New")) {
                if (ImGui::MenuItem("Folder")) {
                    m_NamePopup.Show([this](std::string const& name) {
                        std::error_code ec;
                        if (!std::filesystem::create_directories(m_CurrentPath / name, ec)) {
                            LOG_ERRORF("Failed to create directoried: {}", ec.message());
                        }
                        RefreshContents();
                    });
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Scene")) {
                    m_NamePopup.Show([this](std::string const& name) {
                        auto path = fs::relative(m_CurrentPath, m_Root) / (name + ".fsn");
                        Project::AssetManager()->CreateAsset<Scene>(path);
                        RefreshContents();
                    });
                }
                if (ImGui::MenuItem("PbrMaterial")) {
                    m_NamePopup.Show([this](std::string const& name) {
                        auto path = fs::relative(m_CurrentPath, m_Root) / (name + ".fsn");
                        Project::AssetManager()->CreateAsset<PbrMaterial>(path);
                        RefreshContents();
                    });
                }

                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Show in file explorer")) {
                Dialogs::OpenDirectory(m_CurrentPath);
            }
            ImGui::EndPopup();
        }

        auto item_size = m_Padding + m_ThumbnailSize + CAST(s32, ImGui::GetStyle().FramePadding.x);
        auto columns = CAST(s32, ImGui::GetContentRegionAvail().x / item_size);
        if (columns <= 0)
            columns = 1;

        ImGui::Columns(columns, nullptr, false);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
        ImGui::PushStyleColor(ImGuiCol_Button, Vector4(1.0f, 1.0f, 1.0, 0.1f));

        if (m_CurrentPath != m_Root) {
            Vector2 size(m_ThumbnailSize, m_ThumbnailSize);
            ImGui::ImageButton(style.EditorIcons[EditorIcon::FolderBack]->GetTexture().View, size);
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                ChangeDirectory(m_CurrentPath.parent_path());
            }
            ImGui::TextWrapped("..");
            ImGui::NextColumn();
        }

        // Used to signal a refresh of the content folder,
        // to prevent modifying m_Entries while looping.
        bool refresh { false };
        ImGui::PushFont(EditorStyle::Style().Fonts[EditorFont::BoldSmall]);
        Maybe<std::filesystem::path> requested_directory_change;
        for (auto& entry : m_Entries) {
            ImGui::PushID(entry.Path.c_str());
            defer(ImGui::PopID());
            Vector2 size(m_ThumbnailSize, m_ThumbnailSize);

            if (entry.IsDirectory) {
                ImGui::ImageButton(style.EditorIcons[EditorIcon::Folder]->GetTexture().View, size);
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    requested_directory_change = entry.Path;
                    break;
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET"); payload) {
                        auto handle = *CAST(AssetHandle*, payload->Data);
                        Project::AssetManager()->MoveAsset(handle, entry.Path);
                        refresh = true;
                    }
                    ImGui::EndDragDropTarget();
                }
            } else {
                Texture2D* texture;
                switch (entry.Type) {
                case AssetType::Invalid:
                case AssetType::Image:
                case AssetType::Texture:
                case AssetType::HDRTexture:
                case AssetType::Model:
                case AssetType::Shader: {
                    texture = style.EditorIcons[EditorIcon::GenericAsset].get();
                } break;
                case AssetType::Script: {
                    texture = style.EditorIcons[EditorIcon::Script].get();
                } break;
                case AssetType::PbrMaterial: {
                    texture = style.EditorIcons[EditorIcon::PbrMaterial].get();
                } break;
                case AssetType::Scene: {
                    texture = style.EditorIcons[EditorIcon::Scene].get();
                } break;
                case AssetType::Texture2D: {
                    auto asset = AssetManager::GetAsset<Texture2D>(entry.Metadata.Handle);
                    texture = asset.Get();
                    if (texture == nullptr) {
                        texture = style.EditorIcons[EditorIcon::GenericAsset].get();
                    }
                    size.x = texture->GetMetadata().Aspect() * size.y;
                } break;
                default:
                    PANIC("idk");
                }

                if (m_Selection.contains(entry.ID)) {
                    ImGui::PushStyleColor(ImGuiCol_Button, Color::Coral);
                }

                ImGui::ImageButton(texture->GetTexture().View, size, {}, { 1, 1 }, 0);

                if (m_Selection.contains(entry.ID)) {
                    ImGui::PopStyleColor();
                }

                auto current_font = ImGui::GetFont();
                ImGui::PopFont();
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Open")) {
                        m_Editor->OpenAsset(entry.Metadata.Handle);
                    }
                    if (ImGui::MenuItem("Rename")) {
                        entry.Renaming = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PushFont(current_font);

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    // m_Selection.insert(entry.Id);
                }
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    m_Editor->OpenAsset(entry.Metadata.Handle);
                }
                if (ImGui::BeginDragDropSource()) {
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ASSET", &entry.Metadata.Handle, sizeof(AssetHandle));
                    ImGui::EndDragDropSource();
                }
            }

            if (entry.Renaming) {
                if (ImGui::InputText("", &entry.Name, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    entry.Renaming = false;
                    if (!entry.Name.empty()) {
                        Project::AssetManager()->RenameAsset(entry.ID, entry.Name);
                    }
                    refresh = true;
                }
            } else {
                ImGui::TextWrapped("%s", entry.Name.c_str());
            }
            ImGui::NextColumn();
        }

        if (requested_directory_change.HasValue()) {
            ChangeDirectory(*requested_directory_change);
        }

        if (refresh) {
            RefreshContents();
        }

        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        ImGui::EndChild();
    });
}

// path cannot be a ref because it will point to an entry in m_Entries,
// which we clear before using it.
void ContentBrowserWindow::ChangeDirectory(fs::path const& path) // NOLINT(performance-unnecessary-value-param)
{
    m_CurrentPath = path;
    m_RelativeToRoot = fs::relative(m_CurrentPath, m_Root);
    m_RelativeToRootStringPath = m_RelativeToRoot.string();

    m_Selection.clear();
    m_Entries.clear();
    for (auto const& entry : fs::directory_iterator(path)) {
        auto entry_path = fs::relative(entry.path(), Project::AssetsFolderPath());
        auto metadata = Project::AssetManager()->GetMetadata(entry_path);
        if (metadata.HasValue() || entry.is_directory()) {
            auto meta = metadata.ValueOr({});
            m_Entries.push_back(Entry {
                .ID = meta.Handle,
                .Path = entry.path(),
                .StringPath = entry.path().string(),
                .Name = entry.path().filename().string(),
                .IsDirectory = entry.is_directory(),
                .Type = meta.Type,
                .Metadata = meta,
            });
        }
    }

    std::ranges::sort(m_Entries, [](Entry const& a, Entry const& b) {
        if (a.IsDirectory || b.IsDirectory) {
            return a.IsDirectory && !b.IsDirectory;
        }

        auto to_lowercase = [](std::string const& str) {
            std::string result;
            std::ranges::transform(str, std::back_inserter(result), [](unsigned char c) { return std::tolower(c); });
            return result;
        };

        return std::ranges::lexicographical_compare(to_lowercase(a.Name), to_lowercase(b.Name));
    });
}

void ContentBrowserWindow::RefreshContents()
{
    ChangeDirectory(m_CurrentPath);
}

void ContentBrowserWindow::ImportFile(fs::path const& path)
{
    if (!path.has_filename()) {
        LOG_WARNF("Invalid name for import file: {}", path.string());
        return;
    }
    VERIFY(m_FileTypes.contains(path.extension().string()));

    auto name = path.filename().string();

    auto copy_location = m_CurrentPath / name;
    if (std::error_code err; !fs::copy_file(path, copy_location, fs::copy_options::overwrite_existing, err)) {
        LOG_WARNF(R"(Failed to copy "{}" to "{}": "{}")", path.string(), copy_location.string(), err.message());
        return;
    }

    auto asset_manager = Project::AssetManager();

    auto rel = fs::relative(copy_location, Project::AssetsFolderPath());
    asset_manager->RegisterAsset(rel, m_FileTypes[path.extension().string().c_str()]);
}
