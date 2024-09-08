#include "EditorPCH.h"
#include "ContentBrowser.h"
#include "Serialization/AssetSerializer.h"
#include "ImGuiHelpers.h"
#include "EditorUI.h"
#include "Layers/Editor.h"
#include "Layers/ImGuiLayer.h"
#include "Project/Project.h"
#include "AssetWindows/Texture2DWindow.h"

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/OS/Dialog.h"
#include <Fussion/Util/TextureImporter.h>

#include "imgui.h"
#include "Fussion/Input/Input.h"
#include "Fussion/OS/System.h"

#include <ranges>

using namespace Fussion;
namespace fs = std::filesystem;


void ContentBrowser::NamePopup::show(std::function<void(std::string)> const& callback)
{
    m_callback = callback;
    m_show = true;
    m_opened = true;
}

void ContentBrowser::NamePopup::update()
{
    if (m_show) {
        ImGui::OpenPopup("NamePicker");
        m_show = false;
    }

    bool was_opened = m_opened;
    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    EUI::modal_window("NamePicker", [&] {
        ImGuiH::Text("Please pick a name:");
        ImGui::Separator();

        // Set the keyboard focus here once, when the window first appears.
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
            ImGui::SetKeyboardFocusHere(0);

        if (ImGui::InputText("##input", &m_name)) {
            m_show_error = Project::asset_manager()->is_path_an_asset(m_name, false);
        }

        if (m_show_error) {
            ImGui::Image(EditorStyle::get_style().editor_icons[EditorIcon::Error]->image().view, Vector2(16, 16));
            ImGui::SameLine();
            ImGui::TextUnformatted("Item already exists");
        }

        EUI::button("Accept", [this] {
            accept();
        }, { .disabled = m_show_error });
        ImGui::SameLine();
        EUI::button("Cancel", [&] {
            m_opened = false;

            // Prevent m_Callback from being called.
            was_opened = false;
        });

        if (Input::is_key_pressed(Keys::Enter)) {
            accept();
        }
    }, { .flags = flags, .opened = &m_opened });

    if (was_opened && !m_opened) {
        m_callback(m_name);
        m_callback = nullptr;
        m_name.clear();
    }
}

void ContentBrowser::NamePopup::accept()
{
    if (!m_show_error) {
        m_opened = false;
    }
}

void ContentBrowser::on_start()
{
    m_root = Project::assets_folder();
    change_directory(m_root);

    m_file_types[".png"] = AssetType::Texture2D;
    m_file_types[".jpg"] = AssetType::Texture2D;
    m_file_types[".jpeg"] = AssetType::Texture2D;
    m_file_types[".glb"] = AssetType::Model;
    m_file_types[".gltf"] = AssetType::Model;

    m_import_filter.name = "Supported Asset Files";
    for (auto const& file_type : m_file_types | std::views::keys) {
        m_import_filter.file_patterns.push_back(std::format("*{}", file_type));
    }
}

void ContentBrowser::on_draw()
{
    EUI::window("Content", [&] {

        m_is_focused = ImGui::IsWindowFocused();
        m_name_popup.update();

        EUI::button("Import", [&] {
            for (auto const& path : Fussion::Dialogs::show_file_picker(m_import_filter, true)) {
                import_file(path);
            }
            refresh_contents();
        });
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        ImGuiH::Text("Path: {}", m_relative_to_root_string_path);

        ImGui::SameLine();
        auto width = ImGui::GetContentRegionAvail().x - (16 * 2 + ImGui::GetStyle().FramePadding.x);
        ImGui::Dummy({ width, 0 });

        ImGui::SameLine();

        auto& style = EditorStyle::get_style();
        EUI::image_button(style.editor_icons[EditorIcon::Dots], [&] {
            ImGui::OpenPopup("ContentBrowserOptions");
        }, { .size = Vector2{ 16, 16 } });
        if (ImGui::BeginItemTooltip()) {
            ImGui::TextUnformatted("Press to view content browser options.");
            ImGui::EndTooltip();
        }

        EUI::popup("ContentBrowserOptions", [&] {
            EUI::property("Padding", &m_padding, EUI::PropTypeRange{ .min = 2, .max = 32 });
            EUI::property("Thumbnail Size", &m_thumbnail_size, EUI::PropTypeRange{ .min = 16, .max = 128 });
        });

        ImGui::Separator();

        ImGui::BeginChild("##child");

        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::BeginMenu("New")) {
                if (ImGui::MenuItem("Folder")) {
                    m_name_popup.show([this](std::string const& name) {
                        std::error_code ec;
                        if (!std::filesystem::create_directories(m_current_path / name, ec)) {
                            LOG_ERRORF("Failed to create directoried: {}", ec.message());
                        }
                        refresh_contents();
                    });
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Scene")) {
                    m_name_popup.show([this](std::string const& name) {
                        auto path = fs::relative(m_current_path, m_root) / (name + ".fsn");
                        Project::asset_manager()->create_asset<Scene>(path);
                        refresh_contents();
                    });
                }
                if (ImGui::MenuItem("PbrMaterial")) {
                    m_name_popup.show([this](std::string const& name) {
                        auto path = fs::relative(m_current_path, m_root) / (name + ".fsn");
                        Project::asset_manager()->create_asset<PbrMaterial>(path);
                        refresh_contents();
                    });
                }

                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Show in file explorer")) {
                Dialogs::open_directory(m_current_path);
            }
            ImGui::EndPopup();
        }

        auto item_size = m_padding + m_thumbnail_size + CAST(s32, ImGui::GetStyle().FramePadding.x);
        auto columns = CAST(s32, ImGui::GetContentRegionAvail().x / item_size);
        if (columns <= 0)
            columns = 1;

        ImGui::Columns(columns, nullptr, false);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
        ImGui::PushStyleColor(ImGuiCol_Button, Vector4(1.0f, 1.0f, 1.0, 0.1f));

        if (m_current_path != m_root) {
            Vector2 size(m_thumbnail_size, m_thumbnail_size);
            ImGui::ImageButton(style.editor_icons[EditorIcon::FolderBack]->image().view, size);
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                change_directory(m_current_path.parent_path());
            }
            ImGui::TextWrapped("..");
            ImGui::NextColumn();
        }

        // Used to signal a refresh of the content folder,
        // to prevent modifying m_Entries while looping.
        bool refresh{ false };
        ImGui::PushFont(EditorStyle::get_style().fonts[EditorFont::BoldSmall]);
        Maybe<std::filesystem::path> requested_directory_change;
        for (auto& entry : m_entries) {
            ImGui::PushID(entry.path.c_str());
            defer(ImGui::PopID());
            Vector2 size(m_thumbnail_size, m_thumbnail_size);

            if (entry.is_directory) {
                ImGui::ImageButton(style.editor_icons[EditorIcon::Folder]->image().view, size);
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    requested_directory_change = entry.path;
                    break;
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET"); payload) {
                        auto handle = *CAST(AssetHandle*, payload->Data);
                        Project::asset_manager()->move_asset(handle, entry.path);
                        refresh = true;
                    }
                    ImGui::EndDragDropTarget();
                }
            } else {
                Texture2D* texture;
                switch (entry.type) {
                case AssetType::Invalid:
                case AssetType::Image:
                case AssetType::Texture:
                case AssetType::HDRTexture:
                case AssetType::Model:
                case AssetType::Shader: {
                    texture = style.editor_icons[EditorIcon::GenericAsset].get();
                }
                break;
                case AssetType::Script: {
                    texture = style.editor_icons[EditorIcon::Script].get();
                }
                break;
                case AssetType::PbrMaterial: {
                    texture = style.editor_icons[EditorIcon::PbrMaterial].get();
                }
                break;
                case AssetType::Scene: {
                    texture = style.editor_icons[EditorIcon::Scene].get();
                }
                break;
                case AssetType::Texture2D: {
                    auto asset = AssetManager::get_asset<Texture2D>(entry.metadata.handle);
                    texture = asset.get();
                    if (texture == nullptr) {
                        texture = style.editor_icons[EditorIcon::GenericAsset].get();
                    }
                    size.x = texture->metadata().aspect() * size.y;
                }
                break;
                default:
                    PANIC("idk");
                }

                if (m_selection.contains(entry.id)) {
                    ImGui::PushStyleColor(ImGuiCol_Button, Color::Coral);
                }

                ImGui::ImageButton(texture->image().view, size, {}, { 1, 1 }, 0);

                if (m_selection.contains(entry.id)) {
                    ImGui::PopStyleColor();
                }

                auto current_font = ImGui::GetFont();
                ImGui::PopFont();
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Open")) {
                        m_editor->open_asset(entry.metadata.handle);
                    }
                    if (ImGui::MenuItem("Rename")) {
                        entry.renaming = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PushFont(current_font);

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    // m_Selection.insert(entry.Id);
                }
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    m_editor->open_asset(entry.metadata.handle);
                }
                if (ImGui::BeginDragDropSource()) {
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ASSET", &entry.metadata.handle, sizeof(AssetHandle));
                    ImGui::EndDragDropSource();
                }
            }

            if (entry.renaming) {
                if (ImGui::InputText("", &entry.name, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    entry.renaming = false;
                    if (!entry.name.empty()) {
                        Project::asset_manager()->rename_asset(entry.id, entry.name);
                    }
                    refresh = true;
                }
            } else {
                ImGui::TextWrapped("%s", entry.name.c_str());
            }
            ImGui::NextColumn();
        }

        if (requested_directory_change.has_value()) {
            change_directory(*requested_directory_change);
        }

        if (refresh) {
            refresh_contents();
        }

        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        ImGui::EndChild();
    });
}

// path cannot be a ref because it will point to an entry in m_Entries,
// which we clear before using it.
void ContentBrowser::change_directory(fs::path const& path) // NOLINT(performance-unnecessary-value-param)
{
    m_current_path = path;
    m_relative_to_root = fs::relative(m_current_path, m_root);
    m_relative_to_root_string_path = m_relative_to_root.string();

    m_selection.clear();
    m_entries.clear();
    for (auto const& entry : fs::directory_iterator(path)) {
        auto entry_path = fs::relative(entry.path(), Project::assets_folder());
        auto metadata = Project::asset_manager()->get_metadata(entry_path);
        if (metadata.has_value() || entry.is_directory()) {
            auto meta = metadata.value_or({});
            m_entries.push_back(Entry{
                .id = meta.handle,
                .path = entry.path(),
                .string_path = entry.path().string(),
                .name = entry.path().filename().string(),
                .is_directory = entry.is_directory(),
                .type = meta.type,
                .metadata = meta,
            });
        }
    }

    std::ranges::sort(m_entries, [](Entry const& a, Entry const& b) {
        if (a.is_directory || b.is_directory) {
            return a.is_directory && !b.is_directory;
        }

        auto to_lowercase = [](std::string const& str) {
            std::string result;
            std::ranges::transform(str, std::back_inserter(result), [](unsigned char c) { return std::tolower(c); });
            return result;
        };

        return std::ranges::lexicographical_compare(to_lowercase(a.name), to_lowercase(b.name));
    });
}

void ContentBrowser::refresh_contents()
{
    change_directory(m_current_path);
}

void ContentBrowser::import_file(fs::path const& path)
{
    if (!path.has_filename()) {
        LOG_WARNF("Invalid name for import file: {}", path.string());
        return;
    }
    VERIFY(m_file_types.contains(path.extension().string()));

    auto name = path.filename().string();

    auto copy_location = m_current_path / name;
    if (std::error_code err; !fs::copy_file(path, copy_location, fs::copy_options::overwrite_existing, err)) {
        LOG_WARNF(R"(Failed to copy "{}" to "{}": "{}")", path.string(), copy_location.string(), err.message());
        return;
    }

    auto asset_manager = Project::asset_manager();

    auto rel = fs::relative(copy_location, Project::assets_folder());
    asset_manager->register_asset(rel, m_file_types[path.extension().string().c_str()]);
}
