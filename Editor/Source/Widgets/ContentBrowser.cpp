#include "ContentBrowser.h"
#include "Serialization/AssetSerializer.h"

#include "Assets/Importers/TextureImporter.h"
#include "Layers/ImGuiLayer.h"
#include "Project/Project.h"
#include "imgui.h"

void ContentBrowser::OnStart()
{
    m_Icons[Icon::Folder] = TextureImporter::LoadTextureFromFile("Assets/Icons/Folder.png");
    m_Icons[Icon::GenericAsset] = TextureImporter::LoadTextureFromFile("Assets/Icons/GenericAsset.png");
    m_Icons[Icon::Back] = TextureImporter::LoadTextureFromFile("Assets/Icons/FolderBack.png");

    m_Root = Project::ActiveProject()->GetAssetsFolder();
    ChangeDirectory(m_Root);
}

void ContentBrowser::OnDraw()
{
    if (ImGui::Begin("Content")) {
        m_IsFocused = ImGui::IsWindowFocused();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(m_CurrentPath.string().c_str());
        ImGui::BeginChild("##child");

        static s32 thumbnail_size = 64;
        static s32 padding = 8;

        auto item_size = padding + thumbnail_size + CAST(s32, ImGui::GetStyle().FramePadding.x);
        auto columns = CAST(s32, ImGui::GetContentRegionAvail().x) / item_size;
        if (columns <= 0)
            columns = 1;

        ImGui::Columns(columns, nullptr, false);

        if (m_CurrentPath != m_Root) {
            Vector2 size(thumbnail_size, thumbnail_size);
            size.x = m_Icons[Icon::Back]->Spec().Aspect() * size.y;
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
                size.x = m_Icons[Icon::Folder]->Spec().Aspect() * size.y;
                ImGui::ImageButton(IMGUI_IMAGE(m_Icons[Icon::Folder]->GetImage()), size);

                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    LOG_DEBUGF("Double clicked folder");
                    ChangeDirectory(entry.Path);
                }
            }
            else {
                Ref<Fsn::Texture2D> texture = m_Icons[Icon::GenericAsset];

                auto size = Vector2(64, 64);
                size.x = texture->Spec().Aspect() * size.y;
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
void ContentBrowser::ChangeDirectory(std::filesystem::path path)  // NOLINT(performance-unnecessary-value-param)
{
    m_CurrentPath = path;

    m_Entries.clear();
    for (auto const& entry : std::filesystem::directory_iterator(path)) {
        auto entry_path = std::filesystem::relative(entry.path(), Project::ActiveProject()->GetAssetsFolder());
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