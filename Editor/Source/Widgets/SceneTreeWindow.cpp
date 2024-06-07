#include "SceneTreeWindow.h"

#include <imgui.h>
#include <tracy/Tracy.hpp>

#include "EditorApplication.h"
#include "Engin5/Input/Input.h"
#include "Engin5/Scene/Entity.h"
#include "Engin5/Scene/Components/BaseComponents.h"

void SceneTreeWindow::OnDraw()
{
    ZoneScoped;
    if (ImGui::Begin("Scene Entities")) {
        m_IsFocused = ImGui::IsWindowFocused();

        auto& scene = *Editor::GetActiveScene();

        scene.ForEachEntity([this](Engin5::Entity& entity) {
            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_FramePadding |
                ImGuiTreeNodeFlags_SpanAvailWidth |
                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                ImGuiTreeNodeFlags_OpenOnArrow;

            if (m_Selection.contains(entity.GetId())) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            ImGui::PushID(entity.GetId());
            defer (ImGui::PopID());

            const bool open = ImGui::TreeNodeEx(entity.GetName().c_str(), flags);
            if (ImGui::IsItemClicked()) {
                if (Engin5::Input::IsKeyUp(Engin5::KeyboardKey::LeftControl)) {
                    m_Selection.clear();
                }
                m_Selection[entity.GetId()] = entity;
            }

            if (open) {
                ImGui::TreePop();
            }
        });
    }
    ImGui::End();
}