#include "InspectorWindow.h"

#include "EditorPCH.h"
#include "EditorUI.h"
#include "Fussion/Assets/Model.h"
#include "Fussion/Math/Color.h"
#include "Fussion/Reflection/Attributes.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Components/MeshRenderer.h"
#include "Fussion/Scene/Components/ScriptComponent.h"
#include "ImGuiHelpers.h"
#include "Layers/Editor.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <tracy/Tracy.hpp>

using namespace Fussion;

void InspectorWindow::OnStart()
{
    EditorWindow::OnStart();
}

void InspectorWindow::OnDraw()
{
    ZoneScoped;
    EUI::window("Entity Inspector", [&] {
        m_IsFocused = ImGui::IsWindowFocused();

        if (auto const& selection = Editor::SceneTree().GetSelection(); !selection.empty()) {
            if (selection.size() == 1) {
                auto const handle = selection.begin()->first;
                auto entity = m_Editor->ActiveScene()->GetEntity(handle);
                if (DrawEntity(*entity)) {
                    Editor::ActiveScene()->SetDirty();
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
    bool modified { false };

    auto notify_change = [&component_type, &ptr](std::string_view member_name) {
        for (auto const& method : component_type.get_methods()) {
            if (method.get_metadata().contains("NotifyForAttribute")) {
                auto& notify_attribute = method.get_metadata().at("NotifyForAttribute").as<Attributes::NotifyForAttribute>();
                if (member_name == notify_attribute.MemberName) {
                    auto result = method.try_invoke(ptr);
                    if (result.has_error()) {
                        LOG_ERRORF("Could not invoke method: {}", meta_hpp::get_error_code_message(result.get_error()));
                    }
                }
            }
        }
    };

    auto draw_props = [&] {
        for (auto const& member : component_type.get_members()) {
            auto value = member.get(ptr);
            auto& metadata = member.get_metadata();
            auto& member_name = [&]() -> std::string const& {
                if (metadata.contains("EditorNameAttribute")) {
                    return metadata.at("EditorNameAttribute").as<Attributes::EditorNameAttribute>().Name;
                }
                return member.get_name();
            }();

            if (auto region_attr = metadata.find("RegionAttribute"); region_attr != metadata.end()) {
                auto& region = region_attr->second.as<Attributes::RegionAttribute>();
                (void)region;
                // TODO: Support regions. Appending to the same collapsing header is not possible.
                EUI::property(member_name, [&] {
                    if (DrawProperty(std::move(value), member, ptr)) {
                        notify_change(member_name);
                    }
                });
            } else {
                if (!metadata.contains("vector")) {
                    EUI::property(member_name, [&] {
                        if (DrawProperty(std::move(value), member, ptr)) {
                            notify_change(member_name);
                        }
                    });
                }
            }
        }

        for (auto const& method : component_type.get_methods()) {
            if (method.get_metadata().contains("EditorButtonAttribute")) {
                auto& editor_button = method.get_metadata().at("EditorButtonAttribute").as<Attributes::EditorButtonAttribute>();
                EUI::button(editor_button.ButtonText, [&] {
                    auto result = method.try_invoke(ptr);
                    if (result.has_error()) {
                        LOG_ERRORF("Could not invoke method: {}", meta_hpp::get_error_code_message(result.get_error()));
                    }
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
    EUI::image_button(EditorStyle::Style().EditorIcons[EditorIcon::Dots], [] {
        ImGui::OpenPopup("ComponentSettings");
    },
        { .size = Vector2 { line_height, line_height } });

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
            // TODO: Override this to display the exported script variables.
            draw_props();
        } else if (component_type == meta_hpp::resolve_type<MeshRenderer>()) {
            draw_props();
            if (ImGui::TreeNode("Materials")) {
                for (auto mr = *CAST(MeshRenderer**, ptr.get_data()); auto& material : mr->Materials) {
                    EUI::asset_property(material.meta_poly_ptr().get_type().as_pointer().get_data_type().as_class(), material.meta_poly_ptr());
                }
                ImGui::TreePop();
            }
        } else {
            // Generic case
            draw_props();
        }

        ImGui::Separator();
    }

    return modified;
}

bool InspectorWindow::DrawProperty(meta_hpp::uvalue prop_value, meta_hpp::member const& member, meta_hpp::uvalue& ptr)
{
    ZoneScoped;
    bool modified { false };
    auto const prop_type = prop_value.get_type().as_pointer().get_data_type();
    auto& metadata = member.get_metadata();
    if (prop_type.is_number() && !prop_value.is<bool*>()) {
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
        auto data_ptr = *CAST(void**, prop_value.get_data());
        if (auto range_attr = metadata.find("RangeAttribute"); range_attr != metadata.end()) {
            auto range = range_attr->second.as<Attributes::RangeAttribute>();
            if (ImGui::DragScalar("", type, data_ptr, range.Step, &range.Min, &range.Max)) {
                modified = true;
            }
        } else {
            if (ImGui::DragScalar("", type, data_ptr, 0.1f)) {
                modified = true;
            }
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
        modified |= ImGui::ColorEdit4("", prop_value.as<Color*>()->raw);
    } else if (prop_value.is<Vector2*>()) {
        modified |= ImGui::DragFloat2("", prop_value.as<Vector2*>()->raw);
    } else if (prop_value.is<Vector3*>()) {
        modified |= ImGui::DragFloat3("", prop_value.as<Vector3*>()->raw);
    } else if (prop_value.is<Vector4*>()) {
        modified |= ImGui::DragFloat4("", prop_value.as<Vector4*>()->raw);
    } else if (prop_type.is_class()) {
        auto class_type = prop_type.as_class();
        if (class_type.get_argument_type(1) == meta_hpp::resolve_type<Detail::AssetRefMarker>()) {
            modified |= EUI::asset_property(class_type, std::move(prop_value));
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
    style.Padding = Vector2 { 10, 5 };
    style.Rounding = 1;
    style.Border = true;
    return style;
}

bool InspectorWindow::DrawEntity(Entity& e)
{
    ZoneScoped;

    bool modified { false };
    auto& style = EditorStyle::Style();

    if (ImGui::TreeNode("Debug")) {
        auto m = meta_hpp::resolve_type(e);
        auto parent = m.get_member("m_Parent").get(e).as<Uuid>();
        ImGui::BeginDisabled();

        auto id = e.GetHandle();
        auto local_id = e.SceneLocalID();
        EUI::property("ID", &id);
        EUI::property("LocalID", &local_id);
        EUI::property("Parent", &parent);

        ImGui::EndDisabled();
        ImGui::TreePop();
    }

    modified |= EUI::property("Enabled", e.SetEnabled());
    modified |= EUI::property("Name", &e.Name);

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

    EUI::button("Add Component", [] {
        ImGui::OpenPopup("Popup::AddComponent");
    },
        { .alignment = 0.5f, .override = button_style });

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
