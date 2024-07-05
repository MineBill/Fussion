#include "InspectorWindow.h"
#include "Layers/Editor.h"
#include "ImGuiHelpers.h"
#include "EditorApplication.h"
#include "Fussion/Assets/Mesh.h"
#include "Fussion/Math/Color.h"

#include <imgui.h>
#include <magic_enum/magic_enum.hpp>
#include <misc/cpp/imgui_stdlib.h>
#include <tracy/Tracy.hpp>

#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Component.h"

using namespace Fussion;

std::tuple<f32, f32> ParseRangeMeta(std::string value)
{
    auto comma = value.find_first_of('|');
    auto min = std::stof(value.substr(0, comma));
    auto max = std::stof(value.substr(comma + 1));
    return { min, max };
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

        if (auto const selection = m_Editor->GetSceneTree().GetSelection(); !selection.empty()) {
            if (selection.size() == 1) {
                auto const entity = selection.begin()->second;
                if (DrawEntity(*entity)) {
                    Editor::GetActiveScene().Get()->SetDirty();
                }
            } else {
                ImGui::Text("Multiple entities selected");
            }
        }
    }
    ImGui::End();
}

bool InspectorWindow::DrawComponent(Entity& entity, meta_hpp::class_type component_type, meta_hpp::uvalue ptr)
{
    ZoneScoped;
    bool modified{ false };
    auto const name = component_type.get_metadata().at("Name").as<std::string>();
    if (ImGui::CollapsingHeader(name.c_str())) {
        for (auto const& member : component_type.get_members()) {
            auto value = member.get(ptr);

            ImGui::PushID(member.get_name().data());
            defer(ImGui::PopID());

            DoProperty(member.get_name(), [&] {
                ZoneScoped;
                if (auto const data_type = value.get_type().as_pointer().get_data_type(); data_type.is_number()) {
                    ImGuiDataType type = 0;
                    if (value.is<f32*>()) {
                        type = ImGuiDataType_Float;
                    } else if (value.is<f64*>()) {
                        type = ImGuiDataType_Double;
                    } else if (value.is<u32*>()) {
                        type = ImGuiDataType_U32;
                    } else if (value.is<u64*>()) {
                        type = ImGuiDataType_U64;
                    } else if (value.is<s32*>()) {
                        type = ImGuiDataType_S32;
                    } else if (value.is<s64*>()) {
                        type = ImGuiDataType_S64;
                    } else {
                        PANIC("Unsupported numeric type for member");
                    }
                    // @note value has a pointer to the member pointer, so get_data returns that
                    // pointer to the pointer.
                    if (ImGui::InputScalar("", type, *CAST(void**, value.get_data()))) {
                        modified = true;
                    }
                } else if (value.is<bool*>()) {
                    if (ImGui::Checkbox("", value.as<bool*>())) {
                        modified = true;
                    }
                } else if (value.is<std::string*>()) {
                    if (ImGui::InputText("", value.as<std::string*>())) {
                        modified = true;
                    }
                } else if (value.is<Color*>()) {
                    modified |= ImGui::ColorEdit4("", value.as<Color*>()->Raw);
                } else if (data_type.is_class()) {
                    auto class_type = data_type.as_class();
                    if (class_type.get_argument_type(1) == meta_hpp::resolve_type<Detail::AssetRefMarker>()) {
                        auto m_Handle = class_type.get_member("m_Handle");

                        ImGui::TextUnformatted("Asset Reference:");
                        ImGui::SetNextItemAllowOverlap();
                        Vector2 pos = ImGui::GetCursorPos();
                        ImGui::Button(std::format("{}", CAST(u64, m_Handle.get(value).as<AssetHandle>())).c_str(), Vector2(64, 64));

                        if (ImGui::BeginDragDropTarget()) {
                            auto* payload = ImGui::GetDragDropPayload();
                            if (strcmp(payload->DataType, "CONTENT_BROWSER_ASSET") == 0) {
                                auto handle = CAST(Fussion::AssetHandle*, payload->Data);

                                if (ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                                    m_Handle.set(value, *handle);
                                }
                            }

                            ImGui::EndDragDropTarget();
                        }

                        ImGui::SetCursorPos(pos + Vector2(2, 2));
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(0, 0));
                        if (ImGui::Button("o", Vector2(16, 16))) {

                        }
                        ImGui::PopStyleVar();
                    }
                    // if (class_type.get_arity() > 0) {
                    //     meta_hpp::resolve_type<Fsn::AssetRef<_>>();
                    //     ImGui::Text("Template type!");
                    // } else {
                    //     ImGui::Text("Normal class type!");
                    // }
                    // // if (ImGui::InputText("", value.as<std::string*>())) {
                    // //     modified = true;
                    // // }
                } else {
                    ImGui::Text("Unsupported type for %s", member.get_name().c_str());
                }
            });
        }
        ImGui::Separator();
    }
    // if (ImGui::BeginPopupContextItem()) {
    //     if (ImGui::MenuItem("Remove Component")) {
    //         LOG_WARN("Remove component");
    //         entity.RemoveComponent(component_type);
    //     }
    //
    //     ImGui::EndPopup();
    // }
    return modified;
}

bool InspectorWindow::DrawEntity(Entity& e)
{
    ZoneScoped;

    bool modified{ false };
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

    for (const auto& [id, component] : e.GetComponents()) {
        auto ptr = component->meta_poly_ptr();

        auto type = ptr.get_type().as_pointer().get_data_type().as_class();
        modified |= DrawComponent(e, type, std::move(ptr));
    }

    if (ImGuiH::ButtonCenteredOnLine("Add Component")) {
        ImGui::OpenPopup("Popup::AddComponent");
    }

    if (ImGui::BeginPopup("Popup::AddComponent")) {
        auto scope = meta_hpp::resolve_scope("Components");
        for (auto const& [name, type] : scope.get_typedefs()) {
            VERIFY(type.is_class());
            if (ImGui::MenuItem(name.c_str())) {
                e.AddComponent(type.as_class());
            }
        }
        ImGui::EndPopup();
    }
    return modified;
}

//
// template<std::derived_from<Fussion::Component> T>
// static void DrawComponent(Fsn::Entity& e, auto&& callback) {
//     if (e.HasComponent<T>()) {
//         ZoneScopedN("Drawing Component");
//
//         auto type = meta_hpp::resolve_type<T>();
//         auto name = type.get_metadata().at("Name").template as<std::string>();
//         if (ImGui::CollapsingHeader(name.c_str())) {
//             callback(e.GetComponent<T>());
//             ImGui::Separator();
//         }
//     }
// };
