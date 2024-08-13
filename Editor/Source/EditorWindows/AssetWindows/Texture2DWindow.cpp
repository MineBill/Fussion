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
        VERIFY(settings != nullptr, "Custom asset metadata should have been created for this texture.");

        ImGui::BeginChild("left_panel");
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
        ImGui::BeginChild("right_panel");
        defer (ImGui::EndChild());
        {
            auto asset = AssetManager::GetAsset<Texture2D>(m_AssetHandle);
            if (!asset.IsLoaded()) {
                ImGui::TextUnformatted("Texture is null");
                return;
            }
            auto texture = asset.Get();

            ImGui::Image(IMGUI_IMAGE(texture->GetImage()), size);
        }
    }, { .Style = WindowStyleAssetPreview, .Opened = &m_Opened, .Flags = ImGuiWindowFlags_MenuBar });
}

void Texture2DWindow::OnSave() {}
