#include "InspectorWindow.h"
#include "Layers/Editor.h"
#include "ImGuiHelpers.h"
#include "EditorApplication.h"

#include "Engin5/Scene/Components/BaseComponents.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <tracy/Tracy.hpp>

#include "Engin5/Scene/Component.h"

void InspectorWindow::OnDraw()
{
    ZoneScoped;
    if (ImGui::Begin("Inspector")) {
        m_IsFocused = ImGui::IsWindowFocused();

        auto selection = m_Editor->GetSceneTree().GetSelection();
        if (!selection.empty()) {
            if (selection.size() == 1) {
                auto entity = &selection.begin()->second;
                DrawEntity(*entity);
            } else {
                ImGui::Text("Multiple entities selected");
            }
        }
    }
    ImGui::End();
}

void InspectorWindow::DrawEntity(Engin5::Entity& e)
{
    using namespace Engin5;
    ZoneScoped;
    auto& style = Editor::Get().GetStyle();

    ImGuiH::InputText("Name", e.GetNameRef());

    ImGuiHelpers::BeginGroupPanel("Transform", Vector2(0, 0), style.Fonts.Bold);
    // ImGui::TextUnformatted("Position"); ImGui::SameLine();
    // ImGuiHelpers::DragVec3("Position", &e.GetTransform()->Position, 0.01, 0, 0, "%.2f", style.Fonts.Bold);
    // ImGuiHelpers::DragVec3("Euler Angles", &e.GetTransform()->EulerAngles, 0.01, 0, 0, "%.2f", style.Fonts.Bold);
    ImGuiHelpers::EndGroupPanel();

    for (const auto& [id, component]: e.GetComponents()) {
        ZoneScopedN("Drawing Component");
        auto instance = component->GetTypeInfo();

        if (ImGui::CollapsingHeader(instance.GetType().GetPrettyTypeName().data())) {
            ImGui::Separator();
            for (auto const& member : instance.GetMemberInfosWithFlag("ShowInEditor")) {
                ZoneScopedN("Drawing Component Member");
                ImGuiH::BeginProperty(member.GetName().data());
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(member.GetName().data());

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (member.GetTypeId() == Reflect::TypeId::MakeTypeId<bool>()) {
                    ImGui::Checkbox("##bool", member.GetMemberPointer<bool>());
                }
                else if (member.GetTypeId() == Reflect::TypeId::MakeTypeId<s32>()) {
                    ImGui::InputInt("##s32", member.GetMemberPointer<s32>());
                }
                else if (member.GetTypeId() == Reflect::TypeId::MakeTypeId<s64>()) {
                    ImGui::InputInt("##s64", member.GetMemberPointer<s32>());
                }
                else if (member.GetTypeId() == Reflect::TypeId::MakeTypeId<f32>()) {
                    ImGui::InputFloat("##f32", member.GetMemberPointer<f32>());
                }

                ImGuiH::EndProperty();
            }
            ImGui::Separator();
        }

        for (const auto& function : instance.GetFunctionInfosWithFlag("ShowInEditor")) {
            if (function.IsValid()) {
                if (ImGui::Button(std::format("Invoke '{}'", function.GetName()).c_str()))
                    (void)function.Invoke();
            }
        }
    }

    if (ImGuiH::ButtonCenteredOnLine("Add Component")) {
        ImGui::OpenPopup("Popup::AddComponent");
    }

    if (ImGui::BeginPopup("Popup::AddComponent")) {
        for (auto const& info : Reflect::TypeInfoRegistry::GetAllTypes()) {
            if (info.HasFlag("ShowInEditor")) {
                ImGui::MenuItem(std::format("{}", info.GetType().GetPrettyTypeName()).c_str());
            }
        }
        ImGui::EndPopup();
    }
}