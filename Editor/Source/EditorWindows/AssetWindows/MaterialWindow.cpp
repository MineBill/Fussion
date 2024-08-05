#include "MaterialWindow.h"

#include "EditorUI.h"
#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Project/Project.h"

using namespace Fussion;

void MaterialWindow::OnDraw([[maybe_unused]] f32 delta)
{
    EUI::Window("Material Preview", [&] {
        DrawMenuBar();

        auto material_ref = AssetManager::GetAsset<PbrMaterial>(m_AssetHandle);
        if (!material_ref.IsValid()) {
            ImGui::TextUnformatted("Material instance is null");
            return;
        }
        auto material = material_ref.Get();
        EUI::Property("Object Color", &material->ObjectColor);
        EUI::Property("Metallic", &material->Metallic, EUI::PropTypeRange{ .Min = 0.0, .Max = 1.0 });
        EUI::Property("Roughness", &material->Roughness, EUI::PropTypeRange{ .Min = 0.0, .Max = 1.0 });
        EUI::Property("Albedo", &material->AlbedoMap);
    }, { .Opened = &m_Opened, .Flags = ImGuiWindowFlags_MenuBar });
}

void MaterialWindow::OnSave()
{
    Project::ActiveProject()->GetAssetManager()->SaveAsset(m_AssetHandle);
}
