#include "MaterialWindow.h"

#include "EditorUI.h"
#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Project/Project.h"

using namespace Fussion;

void MaterialWindow::OnDraw([[maybe_unused]] f32 delta)
{
    auto window_name = std::format("Material Preview##{}", m_AssetHandle);
    EUI::Window(window_name, [&] {
        DrawMenuBar();

        auto asset = AssetManager::GetAsset<PbrMaterial>(m_AssetHandle);
        auto material = asset.Get();
        if (!asset.IsLoaded()) {
            ImGui::TextUnformatted("Material instance is null");
            return;
        }
        EUI::Property("Object Color", &material->ObjectColor);
        EUI::Property("Metallic", &material->Metallic, EUI::PropTypeRange{ .Min = 0.0, .Max = 1.0 });
        EUI::Property("Roughness", &material->Roughness, EUI::PropTypeRange{ .Min = 0.0, .Max = 1.0 });

        if (ImGui::CollapsingHeader("Maps", ImGuiTreeNodeFlags_DefaultOpen)) {
            EUI::Property("Albedo Map", &material->AlbedoMap);
            EUI::Property("Normal Map", &material->NormalMap);
            EUI::Property("Ambient Occlusion Map", &material->AmbientOcclusionMap);
            EUI::Property("Metallic Roughness Map", &material->MetallicRoughnessMap);
            EUI::Property("Emissive Map", &material->EmissiveMap);
        }

    }, { .Style = WindowStyleAssetPreview, .Opened = &m_Opened, .Flags = ImGuiWindowFlags_MenuBar });
}

void MaterialWindow::OnSave()
{
    Project::ActiveProject()->GetAssetManager()->SaveAsset(m_AssetHandle);
}
