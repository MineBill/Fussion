#include "Texture2DWindow.h"

#include "EditorUI.h"

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
        if (settings != nullptr) {
            if (EUI::Property("Is Normal Map", &settings->IsNormalMap)) {
                Project::ActiveProject()->GetAssetManager()->RefreshAsset(m_AssetHandle);
            }
        }
        auto texture_ref = AssetManager::GetAsset<Texture2D>(m_AssetHandle);
        if (!texture_ref.IsValid()) {
            ImGui::TextUnformatted("Texture is null");
            return;
        }
        auto texture = texture_ref.Get();

        ImGui::Image(IMGUI_IMAGE(texture->GetImage()), size);
    }, { .Opened = &m_Opened, .Flags = ImGuiWindowFlags_MenuBar });
}

void Texture2DWindow::OnSave() {}
