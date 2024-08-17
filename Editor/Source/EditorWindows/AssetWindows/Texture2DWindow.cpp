#include "Texture2DWindow.h"

#include "EditorUI.h"
#include "Fussion/Assets/AssetManager.h"

using namespace Fussion;

void Texture2DWindow::OnDraw(f32 delta)
{
    ImGui::PushID(m_AssetHandle);
    defer(ImGui::PopID());
    auto window_name = std::format("Texture Preview##{}", m_AssetHandle);
    EUI::Window(window_name, [&] {
        DrawMenuBar();

        auto size = ImGui::GetContentRegionAvail();

        auto settings = AssetManager::GetAssetMetadata<Texture2DMetadata>(m_AssetHandle);
        VERIFY(settings != nullptr, "Custom asset metadata should have been created for this texture.");

        ImGui::BeginChild("texture_properties", Vector2(250, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border);
        {
            auto modified = EUI::Property("Is Normal Map", &settings->IsNormalMap);
            modified |= EUI::Property("Wrapping", &settings->Wrap);
            modified |= EUI::Property("Filter", &settings->Filter);
            modified |= EUI::Property("Format", &settings->Format);
            if (modified) {
                Project::ActiveProject()->GetAssetManager()->RefreshAsset(m_AssetHandle);
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("texture_preview", {}, ImGuiChildFlags_Border);
        {
            auto asset = AssetManager::GetAsset<Texture2D>(m_AssetHandle);
            if (!asset.IsLoaded()) {
                ImGui::TextUnformatted("Texture is null");
                ImGui::EndChild();
                return;
            }
            auto texture = asset.Get();

            size.x = texture->Metadata().Aspect() * size.y;

            ImGui::Image(IMGUI_IMAGE(texture->GetImage()), size, m_UvO, m_Uv1);
        }
        ImGui::EndChild();
    }, { .Style = WindowStyleAssetPreview, .Opened = &m_Opened, .Flags = ImGuiWindowFlags_MenuBar, .Centered = true });
}

void Texture2DWindow::OnSave() {}
