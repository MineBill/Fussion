#include "EditorPCH.h"
#include "InspectorWindow.h"
#include "Layers/Editor.h"
#include "ImGuiHelpers.h"
#include "EditorUI.h"
#include "Fussion/Assets/Model.h"
#include "Fussion/Math/Color.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <tracy/Tracy.hpp>

#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Scene/Components/MeshRenderer.h"
#include "Fussion/Scene/Components/ScriptComponent.h"

using namespace Fussion;

void InspectorWindow::OnStart()
{
    EditorWindow::OnStart();
}

void InspectorWindow::OnDraw()
{
    ZoneScoped;
    EUI::Window("Entity Inspector", [&] {
        m_IsFocused = ImGui::IsWindowFocused();

        if (auto const& selection = Editor::GetSceneTree().GetSelection(); !selection.empty()) {
            if (selection.size() == 1) {
                auto const handle = selection.begin()->first;
                auto entity = m_Editor->GetActiveScene()->GetEntity(handle);
                if (DrawEntity(*entity)) {
                    Editor::GetActiveScene()->SetDirty();
                }
            } else {
                ImGui::Text("Unsupported: Multiple entities selected");
            }
        }
    });
}

bool InspectorWindow::DrawComponent([[maybe_unused]] Entity& entity, meta_hpp::class_type component_type, meta_hpp::uvalue ptr)
{
    ZoneScoped;
    bool modified{ false };

    auto DrawProps = [&] {
        for (auto const& member : component_type.get_members()) {
            auto value = member.get(ptr);

            ImGui::PushID(member.get_name().data());
            defer(ImGui::PopID());

            if (!member.get_metadata().contains("vector")) {
                EUI::Property(member.get_name(), [&] {
                    DrawProperty(std::move(value), member, ptr);
#if 0
                ZoneScoped;
                if (auto const property_type = value.get_type().as_pointer().get_data_type(); property_type.is_number()) {
                    ImGuiDataType type;
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
                } else if (value.is<Vector2*>()) {
                    modified |= ImGui::DragFloat2("", value.as<Vector2*>()->Raw);
                } else if (value.is<Vector3*>()) {
                    modified |= ImGui::DragFloat3("", value.as<Vector3*>()->Raw);
                } else if (value.is<Vector4*>()) {
                    modified |= ImGui::DragFloat4("", value.as<Vector4*>()->Raw);
                } else if (property_type.is_class()) {
                    auto class_type = property_type.as_class();
                    if (class_type.get_argument_type(1) == meta_hpp::resolve_type<Detail::AssetRefMarker>()) {
                        EUI::AssetProperty(class_type, std::move(value));
                    } else if (member.get_metadata().contains("vector")) {
                        // class_type is std::vector<T>
                        auto type_t = class_type.get_argument_type(0);
                        if (type_t == meta_hpp::resolve_type<s32>()) {
                            auto vector = CAST(std::vector<s32>**, value.get_data());
                            if (ImGui::CollapsingHeader("Values")) {
                                // for (auto &number : **vector) {
                                for (auto i = 0; i < (*vector)->size(); i++) {
                                    auto& number = (**vector)[i];
                                    ImGui::PushID(i);
                                    defer(ImGui::PopID());
                                }
                            }

                        }
                    } else {
                        ImGui::Text("Unsupported asset type for %s", member.get_name().c_str());
                    }
                } else if (property_type.is_enum()) {
                    auto enum_type = property_type.as_enum();

                    auto current_evalue = enum_type.value_to_evalue(value);
                    if (ImGui::BeginCombo("", current_evalue.get_name().data())) {
                        for (auto const& evalue : enum_type.get_evalues()) {
                            if (ImGui::Selectable(evalue.get_name().c_str())) {
                                member.set(ptr, evalue.get_value());
                            }
                        }
                        ImGui::EndCombo();
                    }
                } else {
                    ImGui::Text("Unsupported type for %s", member.get_name().c_str());
                }
#endif
                });
            }
        }
    };

    auto const name = component_type.get_metadata().at("Name").as<std::string>();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vector2::Zero);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(15, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    auto opened = ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_AllowOverlap);
    ImGui::PopStyleVar(4);

    auto width = ImGui::GetContentRegionMax().x;

    auto line_height = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;

    ImGui::SameLine(width - line_height * 0.75f);
    ImGui::PushID(CAST(s32, component_type.get_hash()));
    EUI::ImageButton(EditorStyle::GetStyle().EditorIcons[EditorIcon::Dots], [] {
        ImGui::OpenPopup("ComponentSettings");
    }, { .Size = Vector2{ line_height, line_height } });

    if (ImGui::BeginPopupContextItem("ComponentSettings")) {
        if (ImGui::MenuItem("Remove Component")) {
            entity.RemoveComponent(component_type);
        }

        ImGui::EndPopup();
    }
    ImGui::PopID();

    if (opened) {
        // Special case gui for some components.
        if (component_type == meta_hpp::resolve_type<ScriptComponent>()) {
            DrawProps();
        } else if (component_type == meta_hpp::resolve_type<MeshRenderer>()) {
            DrawProps();

            if (ImGui::TreeNode("Materials")) {
                for (auto mr = *CAST(MeshRenderer**, ptr.get_data()); auto& material : mr->Materials) {
                    EUI::AssetProperty(material.meta_poly_ptr().get_type().as_pointer().get_data_type().as_class(), material.meta_poly_ptr());
                }
                ImGui::TreePop();
            }

        } else {
            // Generic case
            DrawProps();
        }

        ImGui::Separator();
    }

    return modified;
}

template<typename... T>
auto DrawVector(meta_hpp::any_type type, meta_hpp::uvalue vector) {}

bool InspectorWindow::DrawProperty(meta_hpp::uvalue prop_value, meta_hpp::member const& member, meta_hpp::uvalue& ptr)
{
    ZoneScoped;
    bool modified{ false };
    if (auto const prop_type = prop_value.get_type().as_pointer().get_data_type(); prop_type.is_number()) {
        ImGuiDataType type;
        if (prop_value.is<f32*>()) {
            type = ImGuiDataType_Float;
        } else if (prop_value.is<f64*>()) {
            type = ImGuiDataType_Double;
        } else if (prop_value.is<u32*>()) {
            type = ImGuiDataType_U32;
        } else if (prop_value.is<u64*>()) {
            type = ImGuiDataType_U64;
        } else if (prop_value.is<s32*>()) {
            type = ImGuiDataType_S32;
        } else if (prop_value.is<s64*>()) {
            type = ImGuiDataType_S64;
        } else {
            PANIC("Unsupported numeric type for member");
        }
        // @note value has a pointer to the member pointer, so get_data returns that
        // pointer to the pointer.
        if (ImGui::InputScalar("", type, *CAST(void**, prop_value.get_data()))) {
            modified = true;
        }
    } else if (prop_value.is<bool*>()) {
        if (ImGui::Checkbox("", prop_value.as<bool*>())) {
            modified = true;
        }
    } else if (prop_value.is<std::string*>()) {
        if (ImGui::InputText("", prop_value.as<std::string*>())) {
            modified = true;
        }
    } else if (prop_value.is<Color*>()) {
        modified |= ImGui::ColorEdit4("", prop_value.as<Color*>()->Raw);
    } else if (prop_value.is<Vector2*>()) {
        modified |= ImGui::DragFloat2("", prop_value.as<Vector2*>()->Raw);
    } else if (prop_value.is<Vector3*>()) {
        modified |= ImGui::DragFloat3("", prop_value.as<Vector3*>()->Raw);
    } else if (prop_value.is<Vector4*>()) {
        modified |= ImGui::DragFloat4("", prop_value.as<Vector4*>()->Raw);
    } else if (prop_type.is_class()) {
        auto class_type = prop_type.as_class();
        if (class_type.get_argument_type(1) == meta_hpp::resolve_type<Detail::AssetRefMarker>()) {
            EUI::AssetProperty(class_type, std::move(prop_value));
            // } else if (member.get_metadata().contains("vector")) {
            // // class_type is std::vector<T>
            // auto type_t = class_type.get_argument_type(0);
            // if (type_t == meta_hpp::resolve_type<s32>()) {
            //     auto vector = CAST(std::vector<s32>**, prop_value.get_data());
            //     if (ImGui::CollapsingHeader("Values")) {
            //         // for (auto &number : **vector) {
            //         for (auto i = 0; i < (*vector)->size(); i++) {
            //             auto& number = (**vector)[i];
            //             DrawProperty(meta_hpp::uvalue(number), {});
            //         }
            //     }
            // }
        } else {
            ImGui::Text("Unsupported asset type for %s", member.get_name().c_str());
        }
    } else if (prop_type.is_enum()) {
        auto enum_type = prop_type.as_enum();

        auto current_evalue = enum_type.value_to_evalue(*prop_value);
        if (current_evalue.is_valid()) {
            if (ImGui::BeginCombo("", current_evalue.get_name().data())) {
                for (auto const& evalue : enum_type.get_evalues()) {
                    if (ImGui::Selectable(evalue.get_name().c_str())) {
                        member.set(ptr, evalue.get_value());
                    }
                }
                ImGui::EndCombo();
            }
        }
    } else {
        ImGui::Text("Unsupported type for %s", member.get_name().c_str());
    }
    return modified;
}

constexpr auto MakeAddComponentButtonStyle() -> ButtonStyle
{
    auto style = ButtonStyle::Default();
    // style.SetButtonColor(Color::FromHex(0x405070FF));
    style.BorderShadowColor = Color::Transparent;
    style.Padding = Vector2{ 10, 5 };
    style.Rounding = 1;
    style.Border = true;
    return style;
}

bool InspectorWindow::DrawEntity(Entity& e)
{
    ZoneScoped;

    bool modified{ false };
    auto& style = EditorStyle::GetStyle();

    if (ImGui::TreeNode("Debug")) {
        auto m = meta_hpp::resolve_type(e);
        auto parent = m.get_member("m_Parent").get(e).as<Uuid>();
        ImGui::BeginDisabled();

        auto id = e.GetId();
        auto local_id = e.GetLocalID();
        EUI::Property("ID", &id);
        EUI::Property("LocalID", &local_id);
        EUI::Property("Parent", &parent);

        ImGui::EndDisabled();
        ImGui::TreePop();
    }

    modified |= EUI::Property("Enabled", e.GetEnabled());
    modified |= EUI::Property("Name", &e.Name);

    ImGuiHelpers::BeginGroupPanel("Transform", Vector2(0, 0), style.Fonts[EditorFont::BoldSmall]);
    ImGui::TextUnformatted("Position");
    modified |= ImGuiHelpers::DragVec3("##position", &e.Transform.Position, 0.01f, 0.f, 0.f, "%.2f", style.Fonts[EditorFont::Bold], style.Fonts[EditorFont::RegularSmall]);

    ImGui::TextUnformatted("Euler Angles");
    modified |= ImGuiHelpers::DragVec3("##euler_angles", &e.Transform.EulerAngles, 0.01f, 0.f, 0.f, "%.2f", style.Fonts[EditorFont::Bold], style.Fonts[EditorFont::RegularSmall]);

    ImGui::TextUnformatted("Scale");
    modified |= ImGuiHelpers::DragVec3("##scale", &e.Transform.Scale, 0.01f, 0.f, 0.f, "%.2f", style.Fonts[EditorFont::Bold], style.Fonts[EditorFont::RegularSmall]);
    ImGuiHelpers::EndGroupPanel();

    for (auto const& component : e.GetComponents() | std::views::values) {
        auto ptr = component->meta_poly_ptr();

        auto type = ptr.get_type().as_pointer().get_data_type().as_class();
        modified |= DrawComponent(e, type, std::move(ptr));
    }

    static constexpr auto button_style = MakeAddComponentButtonStyle();

    EUI::Button("Add Component", [] {
        ImGui::OpenPopup("Popup::AddComponent");
    }, { .Alignment = 0.5f, .Override = button_style });

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
