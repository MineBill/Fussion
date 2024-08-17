#include "SceneTreeWindow.h"
#include "EditorUI.h"

#include <imgui.h>
#include <tracy/Tracy.hpp>

#include "EditorApplication.h"
#include "Fussion/Input/Input.h"
#include "Fussion/Scene/Entity.h"
#include "Fussion/Scene/Components/BaseComponents.h"

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
                        scene->CreateEntity();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Directional Light")) {}
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                ImGui::EndPopup();
            }

            auto& root = scene->GetRoot();
            for (auto const& child : root.GetChildren()) {
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

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

    if (entity->GetChildren().empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }
    if (m_Selection.contains(handle)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::PushID(CAST(s32, CAST(u64, handle)));
    defer(ImGui::PopID());

    auto opened = ImGui::TreeNodeEx(entity->Name.c_str(), flags);

    if (ImGui::IsItemClicked()) {
        SelectEntity(entity->GetId(), Fussion::Input::IsKeyUp(Fussion::Keys::LeftControl));
    }

    if (ImGui::BeginPopupContextItem()) {
        SelectEntity(entity->GetId(), Fussion::Input::IsKeyUp(Fussion::Keys::LeftControl));
        if (ImGui::BeginMenu("New")) {
            if (ImGui::MenuItem("Entity")) {
                scene->CreateEntity("Entity", handle);
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Destroy")) {
            m_Selection.erase(entity->GetId());
            scene->Destroy(entity);
        }
        if (ImGui::MenuItem("Parent to Scene")) {
            entity->SetParent(scene->GetRoot());
        }
        if (ImGui::MenuItem("Align to object")) {
            Editor::GetCamera().EulerAngles = entity->Transform.EulerAngles;
            Editor::GetCamera().Position = entity->Transform.Position;
        }
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
        for (auto const& child : entity->GetChildren()) {
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
