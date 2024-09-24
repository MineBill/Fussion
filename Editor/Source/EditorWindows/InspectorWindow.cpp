#include "EditorPCH.h"
#include "InspectorWindow.h"
#include "Layers/Editor.h"
#include "ImGuiHelpers.h"
#include "EditorUI.h"
#include "Fussion/Assets/Model.h"
#include "Fussion/Math/Color.h"
#include "Fussion/Reflection/Attributes.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <tracy/Tracy.hpp>

#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Scene/Components/MeshRenderer.h"
#include "Fussion/Scene/Components/ScriptComponent.h"

using namespace Fussion;

void InspectorWindow::on_start()
{
    EditorWindow::on_start();
}

void InspectorWindow::on_draw()
{
    ZoneScoped;
    EUI::window("Entity Inspector", [&] {
        m_is_focused = ImGui::IsWindowFocused();

        if (auto const& selection = Editor::scene_tree().selection(); !selection.empty()) {
            if (selection.size() == 1) {
                auto const handle = selection.begin()->first;
                auto entity = m_editor->active_scene()->get_entity(handle);
                if (draw_entity(*entity)) {
                    Editor::active_scene()->set_dirty();
                }
            } else {
                ImGui::Text("Unsupported: Multiple entities selected");
            }
        }
    });
}

bool InspectorWindow::draw_component([[maybe_unused]] Entity& entity, meta_hpp::class_type component_type, meta_hpp::uvalue ptr)
{
    ZoneScoped;
    bool modified{ false };

    auto draw_props = [&] {
        for (auto const& member : component_type.get_members()) {
            auto value = member.get(ptr);
            auto& metadata = member.get_metadata();
            auto& member_name = [&]() -> std::string const& {
                if (metadata.contains("EditorNameAttribute")) {
                    return metadata.at("EditorNameAttribute").as<Attributes::EditorNameAttribute>().name;
                }
                return member.get_name();
            }();

            if (auto region_attr = metadata.find("RegionAttribute"); region_attr != metadata.end()) {
                auto& region = region_attr->second.as<Attributes::RegionAttribute>();
                (void)region;
                // TODO: Support regions. Appending to the same collapsing header is not possible.
                EUI::property(member_name, [&] {
                    draw_property(std::move(value), member, ptr);
                });
            } else {
                if (!metadata.contains("vector")) {
                    EUI::property(member_name, [&] {
                        draw_property(std::move(value), member, ptr);
                    });
                }
            }
        }

        for (auto const& method : component_type.get_methods()) {
            if (method.get_metadata().contains("EditorButtonAttribute")) {
                auto& editor_button = method.get_metadata().at("EditorButtonAttribute").as<Attributes::EditorButtonAttribute>();
                EUI::button(editor_button.button_text, [&] {
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
    EUI::image_button(EditorStyle::get_style().editor_icons[EditorIcon::Dots], [] {
        ImGui::OpenPopup("ComponentSettings");
    }, { .size = Vector2{ line_height, line_height } });

    if (ImGui::BeginPopupContextItem("ComponentSettings")) {
        if (ImGui::MenuItem("Remove Component")) {
            entity.remove_component(component_type);
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
                for (auto mr = *CAST(MeshRenderer**, ptr.get_data()); auto& material : mr->materials) {
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

bool InspectorWindow::draw_property(meta_hpp::uvalue prop_value, meta_hpp::member const& member, meta_hpp::uvalue& ptr)
{
    ZoneScoped;
    bool modified{ false };
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
            if (ImGui::DragScalar("", type, data_ptr, range.step, &range.min, &range.max)) {
                modified = true;
            }
        } else {
            if (ImGui::InputScalar("", type, data_ptr)) {
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
            EUI::asset_property(class_type, std::move(prop_value));
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
    auto style = ButtonStyle::make_default();
    // style.SetButtonColor(Color::FromHex(0x405070FF));
    style.border_shadow_color = Color::Transparent;
    style.padding = Vector2{ 10, 5 };
    style.rounding = 1;
    style.border = true;
    return style;
}

bool InspectorWindow::draw_entity(Entity& e)
{
    ZoneScoped;

    bool modified{ false };
    auto& style = EditorStyle::get_style();

    if (ImGui::TreeNode("Debug")) {
        auto m = meta_hpp::resolve_type(e);
        auto parent = m.get_member("m_Parent").get(e).as<Uuid>();
        ImGui::BeginDisabled();

        auto id = e.handle();
        auto local_id = e.scene_local_id();
        EUI::property("ID", &id);
        EUI::property("LocalID", &local_id);
        EUI::property("Parent", &parent);

        ImGui::EndDisabled();
        ImGui::TreePop();
    }

    modified |= EUI::property("Enabled", e.set_enabled());
    modified |= EUI::property("Name", &e.name);

    ImGuiHelpers::BeginGroupPanel("Transform", Vector2(0, 0), style.fonts[EditorFont::BoldSmall]);
    ImGui::TextUnformatted("Position");
    modified |= ImGuiHelpers::DragVec3("##position", &e.transform.position, 0.01f, 0.f, 0.f, "%.2f", style.fonts[EditorFont::Bold], style.fonts[EditorFont::RegularSmall]);

    ImGui::TextUnformatted("Euler Angles");
    modified |= ImGuiHelpers::DragVec3("##euler_angles", &e.transform.euler_angles, 0.01f, 0.f, 0.f, "%.2f", style.fonts[EditorFont::Bold], style.fonts[EditorFont::RegularSmall]);

    ImGui::TextUnformatted("Scale");
    modified |= ImGuiHelpers::DragVec3("##scale", &e.transform.scale, 0.01f, 0.f, 0.f, "%.2f", style.fonts[EditorFont::Bold], style.fonts[EditorFont::RegularSmall]);
    ImGuiHelpers::EndGroupPanel();

    for (auto const& component : e.get_components() | std::views::values) {
        auto ptr = component->meta_poly_ptr();

        auto type = ptr.get_type().as_pointer().get_data_type().as_class();
        modified |= draw_component(e, type, std::move(ptr));
    }

    static constexpr auto button_style = MakeAddComponentButtonStyle();

    EUI::button("Add Component", [] {
        ImGui::OpenPopup("Popup::AddComponent");
    }, { .alignment = 0.5f, .override = button_style });

    if (ImGui::BeginPopup("Popup::AddComponent")) {
        auto scope = meta_hpp::resolve_scope("Components");
        for (auto const& [name, type] : scope.get_typedefs()) {
            VERIFY(type.is_class());
            if (ImGui::MenuItem(name.c_str())) {
                e.add_component(type.as_class());
            }
        }
        ImGui::EndPopup();
    }
    return modified;
}
