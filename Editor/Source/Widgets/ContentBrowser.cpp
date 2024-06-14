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

        for (auto const& entry : m_Entries) {
            if (entry.IsDirectory) {
                auto size = Vector2(64, 64);
                size.x = m_Icons[Icon::Folder]->Spec().Aspect() * size.y;
                ImGui::ImageButton(IMGUI_IMAGE(m_Icons[Icon::Folder]->GetImage()), size);

                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused()) {
                    LOG_DEBUGF("Double clicked folder");
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
            }

            ImGui::TextUnformatted(entry.Name.c_str());
        }

        ImGui::EndChild();
    }
    ImGui::End();
}

void ContentBrowser::ChangeDirectory(std::filesystem::path const& path)
{
    m_CurrentPath = path;

    m_Entries.clear();
    for (auto const& entry : std::filesystem::directory_iterator(path)) {
        auto metadata = Project::ActiveProject()->GetAssetManager()->GetMetadata(path);
        if (metadata.IsValid() || entry.is_directory()) {
            m_Entries.push_back(Entry {
                .Path = entry.path(),
                .StringPath = entry.path().string(),
                .Name = entry.path().filename().string(),
                .IsDirectory = entry.is_directory(),
                .Type = metadata.Type,
            });
        }

    }
}