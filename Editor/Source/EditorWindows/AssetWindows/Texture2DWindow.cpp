#include "Texture2DWindow.h"

#include "EditorPCH.h"
#include "EditorUI.h"
#include "Fussion/Assets/AssetManager.h"
#include <imgui_internal.h>

// Code is heavily inspired by https://github.com/andyborrell/imgui_tex_inspect

using namespace Fussion;

void Texture2DWindow::on_draw(f32 delta)
{
    (void)delta;

    // auto settings = AssetManager::get_asset_metadata<Texture2DMetadata>(m_asset_handle);
    // VERIFY(settings != nullptr, "Custom asset metadata should have been created for this texture.");
    auto asset = AssetManager::get_asset<Texture2D>(m_asset_handle);
    if (!asset.is_loaded()) {
        ImGui::TextUnformatted("Texture is null");
        ImGui::EndChild();
        return;
    }
    auto texture = asset.get();
    auto& metadata = texture->metadata();

    bool flip = true;
    if (!GPU::is_hdr(metadata.format)) {
        flip = false;
        ImGui::BeginChild("texture_properties", Vector2(250, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border);
        {
            auto modified = EUI::property("Is Normal Map", &metadata.is_normal_map);
            modified |= EUI::property("Format", &metadata.format);
            modified |= EUI::property("Generate Mipmaps", &metadata.generate_mipmaps);
            if (modified) {
                // Project::asset_manager()->refresh_asset(m_asset_handle);
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();
    }

    Vector2 availableSize = ImGui::GetContentRegionAvail();

    Vector2 textureSize = Vector2(metadata.width, metadata.height);
    Vector2 screenSize = textureSize * m_scale;
    Vector2 viewSizeUv = availableSize / screenSize;
    Vector2 viewSize = availableSize;
    Vector2 uv0 = m_pan_position - viewSizeUv * 0.5f;
    Vector2 uv1 = m_pan_position + viewSizeUv * 0.5f;

    if (screenSize.x < availableSize.x) {
        viewSize.x = Math::floor(screenSize.x);
        uv0.x = 0;
        uv1.x = 1;
        m_pan_position.x = 0.5f;
    }

    if (screenSize.y < availableSize.y) {
        viewSize.y = Math::floor(screenSize.y);
        uv0.y = 0;
        uv1.y = 1;
        m_pan_position.y = 0.5f;
    }

    if (flip) {
        viewSizeUv.y *= -1;
        std::swap(uv0.y, uv1.y);
    }

    ImGui::BeginChild("texture_preview", availableSize, 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    {
        ImGui::GetCurrentWindow()->ScrollMax.y = 1.0f;
        auto& gpu_texture = texture->texture();
        auto view = gpu_texture.view;
        ImGui::Image(view, viewSize, uv0, uv1);
        auto& io = ImGui::GetIO();

        bool hovered = ImGui::IsWindowHovered();
        if (!m_is_dragging && hovered && io.MouseClicked[ImGuiMouseButton_Left]) {
            m_is_dragging = true;
        } else if (m_is_dragging) {
            ImVec2 uvDelta = io.MouseDelta * viewSizeUv / viewSize;
            m_pan_position -= Vector2(uvDelta);
            Vector2 abs = Vector2::abs(viewSizeUv);
            m_pan_position = Vector2::max(m_pan_position - abs * 0.5f, Vector2::Zero) + abs * 0.5f;
            m_pan_position = Vector2::min(m_pan_position + abs * 0.5f, Vector2::One) - abs * 0.5f;
        }

        if (m_is_dragging && (io.MouseReleased[ImGuiMouseButton_Left] || !io.MouseDown[ImGuiMouseButton_Left])) {
            m_is_dragging = false;
        }

        if (hovered && io.MouseWheel != 0) {
            constexpr auto minimumGridSize = 4;
            float zoomRate = m_zoom_rate;
            float scale = m_scale.y;
            float prevScale = scale;

            bool keepTexelSizeRegular = scale > minimumGridSize;
            if (io.MouseWheel > 0) {
                scale *= zoomRate;
                if (keepTexelSizeRegular) {
                    // It looks nicer when all the grid cells are the same size
                    // so keep scale integer when zoomed in
                    scale = ImCeil(scale);
                }
            } else {
                scale /= zoomRate;
                if (keepTexelSizeRegular) {
                    // See comment above. We're doing a floor this time to make
                    // sure the scale always changes when scrolling
                    scale = CAST(f32, Math::floor_signed(scale));
                }
            }
            /* To make it easy to get back to 1:1 size we ensure that we stop
             * here without going straight past it*/
            if ((prevScale < 1 && scale > 1) || (prevScale > 1 && scale < 1)) {
                scale = 1;
            }
            m_scale = Vector2(metadata.aspect() * scale, scale);
        }
    }
    ImGui::EndChild();
}

void Texture2DWindow::on_save() { }
