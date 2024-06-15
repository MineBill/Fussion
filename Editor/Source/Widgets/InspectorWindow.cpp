#include "InspectorWindow.h"
#include "Layers/Editor.h"
#include "ImGuiHelpers.h"
#include "EditorApplication.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <tracy/Tracy.hpp>

#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Scripting/ScriptingEngine.h"

std::tuple<f32, f32> ParseRangeMeta(std::string value)
{
    auto comma = value.find_first_of('|');
    auto min = std::stof(value.substr(0, comma));
    auto max = std::stof(value.substr(comma + 1));
    return {min, max};
}

void InspectorWindow::OnStart()
{
    EditorWindow::OnStart();
}

void InspectorWindow::OnDraw()
{
    ZoneScoped;
    if (ImGui::Begin("Inspector")) {
        m_IsFocused = ImGui::IsWindowFocused();

        auto selection = m_Editor->GetSceneTree().GetSelection();
        if (!selection.empty()) {
            if (selection.size() == 1) {
                auto entity = selection.begin()->second;
                DrawEntity(*entity);
            } else {
                ImGui::Text("Multiple entities selected");
            }
        }
    }
    ImGui::End();
}

void InspectorWindow::DrawEntity(Fussion::Entity& e)
{
    using namespace Fussion;
    ZoneScoped;
    auto& style = Editor::Get().GetStyle();

    ImGuiH::InputText("Name", e.GetNameRef());

    ImGuiHelpers::BeginGroupPanel("Transform", Vector2(0, 0), style.Fonts.BoldSmall);
    ImGui::TextUnformatted("Position");
    ImGuiHelpers::DragVec3("##position", &e.Transform.Position, 0.01, 0, 0, "%.2f", style.Fonts.Bold, style.Fonts.RegularSmall);

    ImGui::TextUnformatted("Euler Angles");
    ImGuiHelpers::DragVec3("##euler_angles", &e.Transform.EulerAngles, 0.01, 0, 0, "%.2f", style.Fonts.Bold, style.Fonts.RegularSmall);

    ImGui::TextUnformatted("Scale");
    ImGuiHelpers::DragVec3("##scale", &e.Transform.Scale, 0.01, 0, 0, "%.2f", style.Fonts.Bold, style.Fonts.RegularSmall);
    ImGuiHelpers::EndGroupPanel();

    for (const auto& [id, component]: e.GetComponents()) {
        ZoneScopedN("Drawing Component");
    }

    if (ImGuiH::ButtonCenteredOnLine("Add Component")) {
        ImGui::OpenPopup("Popup::AddComponent");
    }

    if (ImGui::BeginPopup("Popup::AddComponent")) {
        // for (auto const& info : Reflect::TypeInfoRegistry::GetAllTypes()) {
        //     if (info.IsDerivedFrom<Component>() && !e.HasComponent(info.GetTypeId())) {
        //         if (ImGui::MenuItem(std::format("{}", info.GetType().GetPrettyTypeName()).c_str())) {
        //             e.AddComponent(info.GetTypeId());
        //         }
        //     }
        // }
        ImGui::EndPopup();
    }
}
