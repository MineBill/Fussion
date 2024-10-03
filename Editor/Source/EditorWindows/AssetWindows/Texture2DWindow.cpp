#include "Texture2DWindow.h"

#include "EditorPCH.h"
#include "EditorUI.h"
#include "Fussion/Assets/AssetManager.h"

using namespace Fussion;

void Texture2DWindow::OnDraw(f32 delta)
{
    (void)delta;
    auto size = ImGui::GetContentRegionAvail();

    auto settings = AssetManager::GetAssetMetadata<Texture2DMetadata>(m_AssetHandle);
    VERIFY(settings != nullptr, "Custom asset metadata should have been created for this texture.");

    if (!GPU::IsHDR(settings->Format)) {
        ImGui::BeginChild("texture_properties", Vector2(250, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border);
        {
            auto modified = EUI::property("Is Normal Map", &settings->IsNormalMap);
            modified |= EUI::property("Format", &settings->Format);
            modified |= EUI::property("Generate Mipmaps", &settings->GenerateMipmaps);
            if (modified) {
                Project::AssetManager()->RefreshAsset(m_AssetHandle);
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();
    }

    ImGui::BeginChild("texture_preview", {}, ImGuiChildFlags_Border);
    {
        auto asset = AssetManager::GetAsset<Texture2D>(m_AssetHandle);
        if (!asset.IsLoaded()) {
            ImGui::TextUnformatted("Texture is null");
            ImGui::EndChild();
            return;
        }
        auto texture = asset.Get();

        size.x = texture->GetMetadata().Aspect() * size.y;

        ImGui::Image(texture->GetTexture().View, size, m_uv0, m_uv1);
    }
    ImGui::EndChild();
}

void Texture2DWindow::OnSave() { }
