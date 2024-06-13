#include "InspectorWindow.h"
#include "Layers/Editor.h"
#include "ImGuiHelpers.h"
#include "EditorApplication.h"

#include "Engin5/Scene/Components/BaseComponents.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <tracy/Tracy.hpp>

#include "Engin5/Scene/Component.h"
#include "Engin5/Scripting/ScriptingEngine.h"

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

void InspectorWindow::DrawEntity(Engin5::Entity& e)
{
    using namespace Engin5;
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
                if (auto* b = member.GetMemberPointer<bool>(); b) {
                    ImGui::Checkbox("##bool", b);
                }
                else if (auto* number32 = member.GetMemberPointer<s32>()) {
                    ImGui::InputInt("##s32", number32);
                }
                else if (auto* number64 = member.GetMemberPointer<s64>()) {
                    ImGui::InputInt("##s32", transmute(s32*, number64));
                }
                else if (auto* numberf32 = member.GetMemberPointer<f32>()) {
                    if (auto range = member.GetMeta("Range"); range.IsValid()) {
                        auto [min, max] = ParseRangeMeta(range.GetValue());
                        ImGui::SliderFloat("##f32", numberf32, min, max);
                    } else {
                        ImGui::InputFloat("##f32", numberf32);
                    }
                }

                if (auto* string = member.GetMemberPointer<std::string>(); string) {
                    if (member.HasFlag("ScriptRef")) {
                        if (ImGui::Button("Select Script")) {
                        }
                    } else {
                        ImGui::InputText("##string", string);
                    }

                    if constexpr (std::is_destructible_v<int>) {

                    }
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
            if (info.IsDerivedFrom<Component>() && !e.HasComponent(info.GetTypeId())) {
                if (ImGui::MenuItem(std::format("{}", info.GetType().GetPrettyTypeName()).c_str())) {
                    e.AddComponent(info.GetTypeId());
                }
            }
        }
        ImGui::EndPopup();
    }
}
