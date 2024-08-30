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

void SceneTreeWindow::OnDraw()
{
    ZoneScoped;
    ImGuiWindowFlags flags{};

    if (auto& scene = Editor::GetActiveScene()) {
        if (scene->IsDirty()) {
            flags |= ImGuiWindowFlags_UnsavedDocument;
        }
    }

    EUI::Window("Scene Entities", [this] {
        m_IsFocused = ImGui::IsWindowFocused();

        if (auto& scene = Editor::GetActiveScene()) {
            if (ImGui::BeginPopupContextWindow()) {
                if (ImGui::BeginMenu("New")) {
                    if (ImGui::MenuItem("Empty Entity")) {
                        auto entity = scene->CreateEntity();
                        entity->Name = "New Entity";
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Directional Light")) {
                        auto entity = scene->CreateEntity();
                        entity->Name = "Directional Light";
                        entity->AddComponent<Fussion::DirectionalLight>();
                        entity->Transform.EulerAngles.X = 45.0f;
                    }
                    if (ImGui::MenuItem("Camera")) {
                        auto entity = scene->CreateEntity();
                        entity->Name = "Camera";
                        entity->AddComponent<Fussion::Camera>();
                        entity->Transform.EulerAngles.X = 45.0f;
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                ImGui::EndPopup();
            }

            // TODO: Is there a way to prevent the copy here?
            // NOTE: We copy here because the children vector might get
            //       modified if we duplicate an entity.
            auto children = scene->GetRoot().GetChildren();
            for (auto child : children) {
                DrawEntityHierarchy(child);
            }
        } else {
            ImGui::TextUnformatted("No scene loaded");
        }
    }, { .Flags = flags });
}

void SceneTreeWindow::DrawEntityHierarchy(Fsn::Uuid handle)
{
    auto& scene = Editor::GetActiveScene();
    auto entity = scene->GetEntity(handle);

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_OpenOnArrow;

    if (entity->GetChildren().empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (m_Selection.contains(handle)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::PushID(CAST(s32, CAST(u64, handle)));
    defer(ImGui::PopID());

    auto opened = ImGuiH::TreeNode(entity->Name, EditorStyle::GetStyle().EditorIcons[EditorIcon::Entity]->GetImage(), flags);

    if (ImGui::IsItemClicked()) {
        SelectEntity(entity->GetHandle(), Fussion::Input::IsKeyUp(Fussion::Keys::LeftControl));
    }

    if (ImGui::BeginPopupContextItem()) {
        SelectEntity(entity->GetHandle(), Fussion::Input::IsKeyUp(Fussion::Keys::LeftControl));
        if (ImGui::BeginMenu("New")) {
            if (ImGui::MenuItem("Entity")) {
                (void)scene->CreateEntity("Entity", handle);
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate")) {
            auto new_handle = scene->CloneEntity(handle);
            if (new_handle != Fussion::EntityHandle::Invalid) {
                auto* new_entity = scene->GetEntity(new_handle);
                new_entity->Name += " (Clone)";
            }
        }

        if (ImGui::MenuItem("Parent to Scene")) {
            entity->SetParent(scene->GetRoot());
        }

        if (ImGui::MenuItem("Align camera to object")) {
            Editor::GetCamera().EulerAngles = entity->Transform.EulerAngles;
            Editor::GetCamera().Position = entity->Transform.Position;
        }

        if (ImGui::MenuItem("Align object to camera")) {
            entity->Transform.EulerAngles = Editor::GetCamera().EulerAngles;
            entity->Transform.Position = Editor::GetCamera().Position;
        }

        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Text, Color::Red);
        if (ImGui::MenuItem("Destroy")) {
            m_Selection.erase(entity->GetHandle());
            scene->Destroy(*entity);
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
            auto const source = scene->GetEntity(source_handle);
            source->SetParent(*entity);
        }
        ImGui::EndDragDropTarget();
    }

    if (opened) {
        // TODO: Is there a way to prevent the copy here?
        // NOTE: We copy here because the children vector might get
        //       modified if we duplicate an entity.
        auto children = entity->GetChildren();
        for (auto child : entity->GetChildren()) {
            DrawEntityHierarchy(child);
        }
        ImGui::TreePop();
    }
}

void SceneTreeWindow::SelectEntity(Fussion::Uuid entity, bool clear)
{
    if (clear) {
        m_Selection.clear();
    }
    m_Selection[entity] = {};
}
