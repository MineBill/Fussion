#include "InspectorWindow.h"
#include "Layers/Editor.h"
#include "ImGuiHelpers.h"
#include "EditorUI.h"
#include "Fussion/Assets/Mesh.h"
#include "Fussion/Math/Color.h"

#include <imgui.h>
#include <magic_enum/magic_enum.hpp>
#include <misc/cpp/imgui_stdlib.h>
#include <tracy/Tracy.hpp>

#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Scene/Components/ScriptComponent.h"

using namespace Fussion;

void AssetPicker::Update()
{
    if (m_Show) {
        ImGui::OpenPopup("Asset Picker");
        m_Show = false;
    }

    bool was_open = m_Opened;

    EUI::ModalWindow("Asset Picker", [&] {
        ImGuiH::Text("Please pick an asset for '{}':", m_Member.get_name());

        ImGui::Separator();
        for (auto const& handle : m_ViableHandles) {
            EUI::Button(std::format("Asset: {}", handle), [&] {
                m_Member.set(m_Instance, handle);
                m_Opened = false;
            });
        }
    }, { .Flags = ImGuiPopupFlags_None, .Opened = &m_Opened });

    if (was_open && !m_Opened) {
        m_ViableHandles.clear();
    }
}

void AssetPicker::Show(meta_hpp::member const& member, meta_hpp::uvalue const& instance, Fussion::AssetType type)
{
    m_Show = true;
    m_Member = member;
    m_Type = type;
    m_Opened = true;
    m_Instance = instance.copy();

    for (auto const& [handle, metadata] : Project::ActiveProject()->GetAssetManager()->GetRegistry()) {
        if (metadata.Type == type) {
            m_ViableHandles.push_back(handle);
        }
    }
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

        if (auto const& selection = m_Editor->GetSceneTree().GetSelection(); !selection.empty()) {
            if (selection.size() == 1) {
                auto const handle = selection.begin()->first;
                auto entity = m_Editor->GetActiveScene()->GetEntity(handle);
                if (DrawEntity(*entity)) {
                    LOG_DEBUG("Entity was modified, setting scene to dirty");
                    Editor::GetActiveScene()->SetDirty();
                }
            } else {
                ImGui::Text("Unsupported: Multiple entities selected");
            }
        }

        m_AssetPicker.Update();
    }
    ImGui::End();
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
                } else if (value.is<Vector2*>()) {
                    modified |= ImGui::DragFloat2("", value.as<Vector2*>()->Raw);
                } else if (value.is<Vector3*>()) {
                    modified |= ImGui::DragFloat3("", value.as<Vector3*>()->Raw);
                } else if (value.is<Vector4*>()) {
                    modified |= ImGui::DragFloat4("", value.as<Vector4*>()->Raw);
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
                        EUI::ImageButton(EditorStyle::GetStyle().EditorIcons[EditorIcon::Search], Vector2(16, 16), [&] {
                            auto asset_type = class_type.get_method("GetType").invoke(value).as<AssetType>();
                            m_AssetPicker.Show(m_Handle, value, asset_type);
                        });
                        ImGui::PopStyleVar();
                    } else {
                        ImGui::Text("Unsupported type for %s", member.get_name().c_str());
                    }
                } else {
                    ImGui::Text("Unsupported type for %s", member.get_name().c_str());
                }
            });
        }
    };

    auto const name = component_type.get_metadata().at("Name").as<std::string>();
    if (ImGui::CollapsingHeader(name.c_str())) {
        // Special case gui for some components.
        if (component_type == meta_hpp::resolve_type<ScriptComponent>()) {
            if (ImGui::TreeNode("Classes")) {

                ImGui::TreePop();
            }

            if (ImGuiHelpers::ButtonCenteredOnLine("Test")) {
                ptr.as<ScriptComponent*>()->Test();
            }

            DrawProps();
        } else {
            // Generic case
            DrawProps();
        }

        ImGui::Separator();
    }

    return modified;
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
        EUI::Property("ID", &id);
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

    for (const auto& component : e.GetComponents() | std::views::values) {
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
