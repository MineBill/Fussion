#include "Texture2DWindow.h"

#include "EditorUI.h"

using namespace Fussion;

void Texture2DWindow::OnDraw(f32 delta)
{
    EUI::Window("Material Preview", [&] {
        DrawMenuBar();

        auto texture_ref = AssetManager::GetAsset<Texture2D>(m_AssetHandle);
        if (!texture_ref.IsValid()) {
            ImGui::TextUnformatted("Material instance is null");
            return;
        }
        auto texture = texture_ref.Get();

        auto settings = AssetManager::GetAssetSettings<Texture2DMetadata>(m_AssetHandle);
        if (settings != nullptr) {
            EUI::Property("Is Normal Map", &settings->IsNormalMap);
        }

        auto size = ImGui::GetContentRegionAvail();

        ImGui::Image(IMGUI_IMAGE(texture->GetImage()), size);
    }, { .Opened = &m_Opened, .Flags = ImGuiWindowFlags_MenuBar });
}

void Texture2DWindow::OnSave() {}
