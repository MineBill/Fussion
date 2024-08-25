#include "EditorPCH.h"
#include "MaterialWindow.h"

#include "EditorUI.h"
#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Project/Project.h"

using namespace Fussion;

void MaterialWindow::OnDraw([[maybe_unused]] f32 delta)
{
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
}

void MaterialWindow::OnSave()
{
    Project::GetAssetManager()->SaveAsset(m_AssetHandle);
}
