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
    EUI::property("Object Color", &material->object_color);
    EUI::property("Metallic", &material->metallic, EUI::PropTypeRange { .min = 0.0, .max = 1.0 });
    EUI::property("Roughness", &material->roughness, EUI::PropTypeRange { .min = 0.0, .max = 1.0 });

    if (EUI::property("Tiling", &material->tiling)) {
        // material->update_sampler();
    }
    if (ImGui::CollapsingHeader("Maps", ImGuiTreeNodeFlags_DefaultOpen)) {
        EUI::property("Albedo Map", &material->albedo_map);
        EUI::property("Normal Map", &material->normal_map);
        EUI::property("Ambient Occlusion Map", &material->ambient_occlusion_map);
        EUI::property("Metallic Roughness Map", &material->metallic_roughness_map);
        EUI::property("Emissive Map", &material->emissive_map);
    }
}

void MaterialWindow::OnSave()
{
    Project::AssetManager()->SaveAsset(m_AssetHandle);
}
