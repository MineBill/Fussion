#include "ContentBrowser.h"
#include "Serialization/AssetSerializer.h"

#include "Assets/Importers/TextureImporter.h"
#include "Layers/ImGuiLayer.h"
#include "Project/Project.h"
#include "imgui.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/OS/Dialog.h"

#include <ranges>

using namespace Fussion;
namespace fs = std::filesystem;

void ContentBrowser::OnStart()
{
    m_Icons[Icon::Folder] = TextureImporter::LoadTextureFromFile("Assets/Icons/Folder.png");
    m_Icons[Icon::GenericAsset] = TextureImporter::LoadTextureFromFile("Assets/Icons/GenericAsset.png");
    m_Icons[Icon::Back] = TextureImporter::LoadTextureFromFile("Assets/Icons/FolderBack.png");

    m_Root = Project::ActiveProject()->GetAssetsFolder();
    ChangeDirectory(m_Root);

    m_FileTypes[".png"] = AssetType::Texture2D;
    m_FileTypes[".jpg"] = AssetType::Texture2D;
    m_FileTypes[".jpeg"] = AssetType::Texture2D;
    m_FileTypes[".glb"] = AssetType::Mesh;
    m_FileTypes[".gltf"] = AssetType::Mesh;

    m_ImportFilter.Name = "Supported Asset Files";
    for (const auto& file_type : m_FileTypes | std::views::keys) {
        m_ImportFilter.FilePatterns.push_back(std::format("*{}", file_type));
    }
}

void ContentBrowser::OnDraw()
{
    if (ImGui::Begin("Content")) {
        m_IsFocused = ImGui::IsWindowFocused();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(m_CurrentPath.string().c_str());
        if (ImGui::Button("Import")) {
            auto file = Fussion::Dialogs::ShowFilePicker(m_ImportFilter);
            ImportFile(file);
            Refresh();
        }
        ImGui::BeginChild("##child");

        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::BeginMenu("New")) {
                if (ImGui::MenuItem("PbrMaterial")) {
                    auto path = fs::relative(m_CurrentPath, m_Root) / "New PbrMaterial.fsn";
                    Project::ActiveProject()->GetAssetManager()->CreateAsset<PbrMaterial>(path);
                    Refresh();
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        static s32 thumbnail_size = 64;
        static s32 padding = 8;

        auto item_size = padding + thumbnail_size + CAST(s32, ImGui::GetStyle().FramePadding.x);
        auto columns = CAST(s32, ImGui::GetContentRegionAvail().x) / item_size;
        if (columns <= 0)
            columns = 1;

        ImGui::Columns(columns, nullptr, false);

        if (m_CurrentPath != m_Root) {
            Vector2 size(thumbnail_size, thumbnail_size);
            size.X = m_Icons[Icon::Back]->Spec().Aspect() * size.Y;
            ImGui::ImageButton(IMGUI_IMAGE(m_Icons[Icon::Back]->GetImage()), size);
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                LOG_DEBUGF("Double clicked folder");
                ChangeDirectory(m_CurrentPath.parent_path());
            }
        }

        for (auto const& entry : m_Entries) {
            ImGui::PushID(entry.Path.c_str());
            defer (ImGui::PopID());
            Vector2 size(thumbnail_size, thumbnail_size);
            if (entry.IsDirectory) {
                size.X = m_Icons[Icon::Folder]->Spec().Aspect() * size.Y;
                ImGui::ImageButton(IMGUI_IMAGE(m_Icons[Icon::Folder]->GetImage()), size);

                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    LOG_DEBUGF("Double clicked folder");
                    ChangeDirectory(entry.Path);
                }
            }
            else {
                Ref<Fsn::Texture2D> texture = m_Icons[Icon::GenericAsset];

                size.X = texture->Spec().Aspect() * size.Y;
                ImGui::ImageButton(IMGUI_IMAGE(texture->GetImage()), size);

                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    LOG_DEBUGF("Double clicked asset");
                }
                if (ImGui::BeginDragDropSource()) {
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ASSET", &entry.Metadata.Handle, sizeof(Fussion::AssetHandle));
                    ImGui::EndDragDropSource();
                }
            }

            ImGui::TextUnformatted(entry.Name.c_str());
            ImGui::NextColumn();
        }

        ImGui::EndChild();
    }
    ImGui::End();
}

// path cannot be a ref because it will point to an entry in m_Entries,
// which we clear before using it.
void ContentBrowser::ChangeDirectory(fs::path path)  // NOLINT(performance-unnecessary-value-param)
{
    m_CurrentPath = path;

    m_Entries.clear();
    for (auto const& entry : fs::directory_iterator(path)) {
        auto entry_path = fs::relative(entry.path(), Project::ActiveProject()->GetAssetsFolder());
        auto metadata = Project::ActiveProject()->GetAssetManager()->GetMetadata(entry_path);
        if (metadata.IsValid() || entry.is_directory()) {
            m_Entries.push_back(Entry {
                .Path = entry.path(),
                .StringPath = entry.path().string(),
                .Name = entry.path().filename().string(),
                .IsDirectory = entry.is_directory(),
                .Type = metadata.Type,
                .Metadata = metadata,
            });
        }

    }
}

void ContentBrowser::Refresh()
{
    ChangeDirectory(m_CurrentPath);
}

void ContentBrowser::ImportFile(fs::path const& path)
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

    auto asset_manager = Project::ActiveProject()->GetAssetManager();

    auto rel = fs::relative(copy_location, Project::ActiveProject()->GetAssetsFolder());
    asset_manager->RegisterAsset(rel, m_FileTypes[path.extension().string().c_str()]);
}
