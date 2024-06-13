#include "SceneTreeWindow.h"

#include <imgui.h>
#include <tracy/Tracy.hpp>

#include "EditorApplication.h"
#include "Fussion/Input/Input.h"
#include "Fussion/Scene/Entity.h"
#include "Fussion/Scene/Components/BaseComponents.h"

void SceneTreeWindow::OnDraw()
{
    ZoneScoped;
    if (ImGui::Begin("Scene Entities")) {
        m_IsFocused = ImGui::IsWindowFocused();

        if (auto scene_ref = Editor::GetActiveScene()) {
            auto scene = scene_ref.Get();

            if (ImGui::BeginPopupContextWindow()) {
                if (ImGui::BeginMenu("New")) {
                    if (ImGui::MenuItem("Entity")) {
                        scene->CreateEntity();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            scene->ForEachEntity([this](Fussion::Entity& entity) {
                ImGuiTreeNodeFlags flags =
                    ImGuiTreeNodeFlags_FramePadding |
                    ImGuiTreeNodeFlags_SpanAvailWidth |
                    ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    ImGuiTreeNodeFlags_OpenOnArrow;

                if (m_Selection.contains(entity.GetId())) {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                ImGui::PushID(entity.GetId());
                defer(ImGui::PopID());

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vector2(2, 2));
                const bool open = ImGui::TreeNodeEx(entity.GetName().c_str(), flags);
                if (ImGui::IsItemClicked()) {
                    if (Fussion::Input::IsKeyUp(Fussion::KeyboardKey::LeftControl)) {
                        m_Selection.clear();
                    }
                    m_Selection[entity.GetId()] = &entity;
                }

                if (open) {
                    ImGui::TreePop();
                }
                ImGui::PopStyleVar();
            });
        } else {
            ImGui::TextUnformatted("No scene loaded");
        }
    }
    ImGui::End();
}