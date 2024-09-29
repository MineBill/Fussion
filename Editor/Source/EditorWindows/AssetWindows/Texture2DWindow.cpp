#include "Texture2DWindow.h"

#include "EditorPCH.h"
#include "EditorUI.h"
#include "Fussion/Assets/AssetManager.h"

using namespace Fussion;

void Texture2DWindow::on_draw(f32 delta)
{
    (void)delta;
    auto size = ImGui::GetContentRegionAvail();

    auto settings = AssetManager::get_asset_metadata<Texture2DMetadata>(m_asset_handle);
    VERIFY(settings != nullptr, "Custom asset metadata should have been created for this texture.");

    if (!GPU::is_hdr(settings->format)) {
        ImGui::BeginChild("texture_properties", Vector2(250, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border);
        {
            auto modified = EUI::property("Is Normal Map", &settings->is_normal_map);
            modified |= EUI::property("Format", &settings->format);
            modified |= EUI::property("Generate Mipmaps", &settings->generate_mipmaps);
            if (modified) {
                Project::asset_manager()->refresh_asset(m_asset_handle);
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();
    }

    ImGui::BeginChild("texture_preview", {}, ImGuiChildFlags_Border);
    {
        auto asset = AssetManager::get_asset<Texture2D>(m_asset_handle);
        if (!asset.is_loaded()) {
            ImGui::TextUnformatted("Texture is null");
            ImGui::EndChild();
            return;
        }
        auto texture = asset.get();

        size.x = texture->metadata().aspect() * size.y;

        ImGui::Image(texture->image().view, size, m_uv0, m_uv1);
    }
    ImGui::EndChild();
}

void Texture2DWindow::on_save() { }
