#include "Texture2DWindow.h"

#include "EditorPCH.h"
#include "EditorUI.h"
#include "Fussion/Assets/AssetManager.h"
#include <imgui_internal.h>

// Code is heavily inspired by https://github.com/andyborrell/imgui_tex_inspect

using namespace Fussion;

void Texture2DWindow::OnDraw(f32 delta)
{
    (void)delta;
    auto size = ImGui::GetContentRegionAvail();

    auto settings = AssetManager::GetAssetMetadata<Texture2DMetadata>(m_AssetHandle);
    VERIFY(settings != nullptr, "Custom asset metadata should have been created for this texture.");

    bool flip = true;
    if (!GPU::IsHDR(settings->Format)) {
        flip = false;
        ImGui::BeginChild("texture_properties", Vector2(250, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border);
        {
            auto modified = EUI::Property("Is Normal Map", &settings->IsNormalMap);
            modified |= EUI::Property("Format", &settings->Format);
            modified |= EUI::Property("Generate Mipmaps", &settings->GenerateMipmaps);
            if (modified) {
                Project::AssetManager()->RefreshAsset(m_AssetHandle);
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();
    }

    auto asset = AssetManager::GetAsset<Texture2D>(m_AssetHandle);
    if (!asset.IsLoaded()) {
        ImGui::TextUnformatted("Texture is null");
        ImGui::EndChild();
        return;
    }
    auto texture = asset.Get();
    auto& metadata = texture->GetMetadata();
    Vector2 availableSize = ImGui::GetContentRegionAvail();

    Vector2 textureSize = Vector2(metadata.Width, metadata.Height);
    Vector2 screenSize = textureSize * m_Scale;
    Vector2 viewSizeUv = availableSize / screenSize;
    Vector2 viewSize = availableSize;
    Vector2 uv0 = m_PanPosition - viewSizeUv * 0.5f;
    Vector2 uv1 = m_PanPosition + viewSizeUv * 0.5f;

    if (screenSize.x < availableSize.x) {
        viewSize.x = Math::Floor(screenSize.x);
        uv0.x = 0;
        uv1.x = 1;
        m_PanPosition.x = 0.5f;
    }

    if (screenSize.y < availableSize.y) {
        viewSize.y = Math::Floor(screenSize.y);
        uv0.y = 0;
        uv1.y = 1;
        m_PanPosition.y = 0.5f;
    }

    if (flip) {
        viewSizeUv.y *= -1;
        std::swap(uv0.y, uv1.y);
    }

    ImGui::BeginChild("texture_preview", availableSize, 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    {
        ImGui::GetCurrentWindow()->ScrollMax.y = 1.0f;
        ImGui::Image(texture->GetTexture().View, viewSize, uv0, uv1);
        auto& io = ImGui::GetIO();

        bool hovered = ImGui::IsWindowHovered();
        if (!m_IsDragging && hovered && io.MouseClicked[ImGuiMouseButton_Left]) {
            m_IsDragging = true;
        } else if (m_IsDragging) {
            ImVec2 uvDelta = io.MouseDelta * viewSizeUv / viewSize;
            m_PanPosition -= Vector2(uvDelta);
            Vector2 abs = Vector2::Abs(viewSizeUv);
            m_PanPosition = Vector2::Max(m_PanPosition - abs * 0.5f, Vector2::Zero) + abs * 0.5f;
            m_PanPosition = Vector2::Min(m_PanPosition + abs * 0.5f, Vector2::One) - abs * 0.5f;
        }

        if (m_IsDragging && (io.MouseReleased[ImGuiMouseButton_Left] || !io.MouseDown[ImGuiMouseButton_Left])) {
            m_IsDragging = false;
        }

        if (hovered && io.MouseWheel != 0) {
            constexpr auto minimumGridSize = 4;
            float zoomRate = m_ZoomRate;
            float scale = m_Scale.y;
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
                    scale = CAST(f32, Math::FloorSigned(scale));
                }
            }
            /* To make it easy to get back to 1:1 size we ensure that we stop
             * here without going straight past it*/
            if ((prevScale < 1 && scale > 1) || (prevScale > 1 && scale < 1)) {
                scale = 1;
            }
            m_Scale = Vector2(metadata.Aspect() * scale, scale);
        }
    }
    ImGui::EndChild();
}

void Texture2DWindow::OnSave() { }
