#include "MaterialWindow.h"

#include "EditorPCH.h"
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
    EUI::Property("Object Color", &material->object_color);
    EUI::Property("Metallic", &material->metallic, EUI::PropTypeRange { .Min = 0.0, .Max = 1.0 });
    EUI::Property("Roughness", &material->roughness, EUI::PropTypeRange { .Min = 0.0, .Max = 1.0 });

    if (EUI::Property("Tiling", &material->tiling)) {
        // material->update_sampler();
    }
    if (ImGui::CollapsingHeader("Maps", ImGuiTreeNodeFlags_DefaultOpen)) {
        EUI::Property("Albedo Map", &material->albedo_map);
        EUI::Property("Normal Map", &material->normal_map);
        EUI::Property("Ambient Occlusion Map", &material->ambient_occlusion_map);
        EUI::Property("Metallic Roughness Map", &material->metallic_roughness_map);
        EUI::Property("Emissive Map", &material->emissive_map);
    }
}

void MaterialWindow::OnSave()
{
    Project::AssetManager()->SaveAsset(m_AssetHandle);
}
