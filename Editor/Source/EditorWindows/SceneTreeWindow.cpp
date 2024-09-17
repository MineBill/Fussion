#include "EditorPCH.h"
#include "SceneTreeWindow.h"
#include "EditorUI.h"
#include "ImGuiHelpers.h"

#include "EditorApplication.h"
#include "Fussion/Input/Input.h"
#include "Fussion/Scene/Entity.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Fussion/Scene/Components/Camera.h"
#include "Fussion/Scene/Components/DirectionalLight.h"

#include <imgui.h>
#include <tracy/Tracy.hpp>

void SceneTreeWindow::on_draw()
{
    ZoneScoped;
    ImGuiWindowFlags flags{};

    if (auto& scene = Editor::active_scene()) {
        if (scene->is_dirty()) {
            flags |= ImGuiWindowFlags_UnsavedDocument;
        }
    }

    EUI::window("Scene Entities", [this] {
        m_is_focused = ImGui::IsWindowFocused();

        if (auto& scene = Editor::active_scene()) {
            if (ImGui::BeginPopupContextWindow()) {
                if (ImGui::BeginMenu("New")) {
                    if (ImGui::MenuItem("Empty Entity")) {
                        auto entity = scene->create_entity();
                        entity->name = "New Entity";
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Directional Light")) {
                        auto entity = scene->create_entity();
                        entity->name = "Directional Light";
                        entity->add_component<Fussion::DirectionalLight>();
                        entity->transform.euler_angles.x = 45.0f;
                    }
                    if (ImGui::MenuItem("Camera")) {
                        auto entity = scene->create_entity();
                        entity->name = "Camera";
                        entity->add_component<Fussion::Camera>();
                        entity->transform.euler_angles.x = 45.0f;
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                ImGui::EndPopup();
            }

            // TODO: Is there a way to prevent the copy here?
            // NOTE: We copy here because the children vector might get
            //       modified if we duplicate an entity.
            auto children = scene->root().children();
            for (auto child : children) {
                draw_entity_hierarchy(child);
            }
        } else {
            ImGui::TextUnformatted("No scene loaded");
        }
    }, { .flags = flags });
}

void SceneTreeWindow::draw_entity_hierarchy(Fsn::Uuid handle)
{
    auto& scene = Editor::active_scene();
    auto entity = scene->get_entity(handle);

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_OpenOnArrow;

    if (entity->children().empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (m_selection.contains(handle)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::PushID(CAST(s32, CAST(u64, handle)));
    defer(ImGui::PopID());

    auto opened = ImGuiH::TreeNode(entity->name, EditorStyle::get_style().editor_icons[EditorIcon::Entity]->image().view, flags);

    if (ImGui::IsItemClicked()) {
        select_entity(entity->handle(), Fussion::Input::is_key_up(Fussion::Keys::LeftControl));
    }

    if (ImGui::BeginPopupContextItem()) {
        select_entity(entity->handle(), Fussion::Input::is_key_up(Fussion::Keys::LeftControl));
        if (ImGui::BeginMenu("New")) {
            if (ImGui::MenuItem("Entity")) {
                (void)scene->create_entity("Entity", handle);
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate")) {
            auto new_handle = scene->clone_entity(handle);
            if (new_handle != Fussion::EntityHandle::Invalid) {
                auto* new_entity = scene->get_entity(new_handle);
                new_entity->name += " (Clone)";
            }
        }

        if (ImGui::MenuItem("Parent to Scene")) {
            entity->set_parent(scene->root());
        }

        if (ImGui::MenuItem("Align camera to object")) {
            Editor::camera().euler_angles = entity->transform.euler_angles;
            Editor::camera().position = entity->transform.position;
        }

        if (ImGui::MenuItem("Align object to camera")) {
            entity->transform.euler_angles = Editor::camera().euler_angles;
            entity->transform.position = Editor::camera().position;
        }

        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Text, Color::Red);
        if (ImGui::MenuItem("Destroy")) {
            m_selection.erase(entity->handle());
            scene->destroy(*entity);
        }
        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }

    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("SCENE_TREE_NODE", &handle, sizeof(handle), ImGuiCond_Once);
        ImGui::TextUnformatted("Drop over another tree node to re-parent.");
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (auto const payload = ImGui::AcceptDragDropPayload("SCENE_TREE_NODE"); payload != nullptr) {
            Fsn::Uuid const source_handle = *CAST(Fsn::Uuid*, payload->Data);
            auto const source = scene->get_entity(source_handle);
            source->set_parent(*entity);
        }
        ImGui::EndDragDropTarget();
    }

    if (opened) {
        // TODO: Is there a way to prevent the copy here?
        // NOTE: We copy here because the children vector might get
        //       modified if we duplicate an entity.
        auto children = entity->children();
        for (auto child : children) {
            draw_entity_hierarchy(child);
        }
        ImGui::TreePop();
    }
}

void SceneTreeWindow::select_entity(Fussion::Uuid entity, bool clear)
{
    if (clear) {
        m_selection.clear();
    }
    m_selection[entity] = {};
}
