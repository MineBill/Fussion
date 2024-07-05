#include "ViewportWindow.h"
#include "Layers/Editor.h"
#include "EditorApplication.h"
#include "SceneRenderer.h"
#include "Layers/ImGuiLayer.h"
#include "Fussion/Assets/AssetManager.h"

#include <cmath>
#include <imgui.h>
#include "ImGuizmo.h"
#include "Fussion/Events/KeyboardEvents.h"

#include <tracy/Tracy.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <magic_enum/magic_enum.hpp>

using namespace Fussion;

bool operator==(const Vector2& vec, const Vector2& rhs)
{
    return fabs(vec.X - rhs.X) <= FLT_EPSILON && fabs(vec.Y - rhs.Y) <= FLT_EPSILON;
}

ImGuizmo::MODE GizmoSpaceToImGuizmo(ViewportWindow::GizmoSpace space)
{
    switch (space) {
    case ViewportWindow::GizmoSpace::Local:
        return ImGuizmo::LOCAL;
    case ViewportWindow::GizmoSpace::World:
        return ImGuizmo::WORLD;
    }
    UNREACHABLE;
}

ImGuizmo::OPERATION GizmoModeToImGuizmo(ViewportWindow::GizmoMode mode)
{
    switch (mode) {
    case ViewportWindow::GizmoMode::Translation:
        return ImGuizmo::TRANSLATE;
    case ViewportWindow::GizmoMode::Rotation:
        return ImGuizmo::ROTATE;
    case ViewportWindow::GizmoMode::Scale:
        return ImGuizmo::SCALE;
    }
    UNREACHABLE;
}

void ViewportWindow::OnDraw()
{
    ZoneScoped;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(0, 0));
    defer(ImGui::PopStyleVar());
    if (ImGui::Begin("Viewport")) {
        m_IsFocused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
        m_ContentOriginScreen = ImGui::GetCursorScreenPos();

        if (const auto size = ImGui::GetContentRegionAvail(); m_Size != size) {
            Editor::OnViewportResized(size);
            m_Size = size;
        }

        Vector2 origin = ImGui::GetCursorPos();

        {
            ZoneScopedN("Get image from set");
            const auto image = Editor::Get().GetSceneRenderer().GetFrameBuffer()->GetColorAttachment(1);
            ImGui::Image(IMGUI_IMAGE(image), m_Size);
        }

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(5, 5));
        defer(ImGui::PopStyleVar());
        if (ImGui::Begin("Debug Overlay", nullptr, window_flags)) {
            ImGui::TextUnformatted("Debug Overlay");
            ImGui::Separator();
            auto pos = Editor::GetCamera().Position;
            ImGuiH::Text("Mouse Position: {}", pos);
        }
        ImGui::End();

        if (ImGui::BeginDragDropTarget()) {
            auto* payload = ImGui::GetDragDropPayload();
            if (strcmp(payload->DataType, "CONTENT_BROWSER_ASSET") == 0) {
                auto handle = CAST(AssetHandle*, payload->Data);

                auto metadata = Project::ActiveProject()->GetAssetManager()->GetMetadata(*handle);
                if (metadata.Type == AssetType::Scene) {
                    if (ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                        auto scene = AssetManager::GetAsset<Scene>(*handle);
                        Editor::ChangeScene(scene);
                    }
                } else if (metadata.Type == AssetType::Texture2D) {
                    if (ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                        Editor::Get().TextureRef = AssetManager::GetAsset<Texture2D>(*handle);
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::SetCursorPos(origin + Vector2(5, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);

        if (ImGuiH::Button("Gizmo Mode: {}", magic_enum::enum_name(m_GizmoMode))) {
            ImGui::OpenPopup("GizmoSelection");
        }

        if (ImGui::BeginPopup("GizmoSelection")) {
            if (ImGui::MenuItem("Translation", "W", m_GizmoMode == GizmoMode::Translation)) {
                m_GizmoMode = GizmoMode::Translation;
            }
            if (ImGui::MenuItem("Rotation", "E", m_GizmoMode == GizmoMode::Rotation)) {
                m_GizmoMode = GizmoMode::Rotation;
            }
            if (ImGui::MenuItem("Scale", "R", m_GizmoMode == GizmoMode::Scale)) {
                m_GizmoMode = GizmoMode::Scale;
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        if (ImGuiH::Button("Gizmo Space: {}", magic_enum::enum_name(m_GizmoSpace))) {
            if (m_GizmoSpace == GizmoSpace::Local) {
                m_GizmoSpace = GizmoSpace::World;
            } else if (m_GizmoSpace == GizmoSpace::World) {
                m_GizmoSpace = GizmoSpace::Local;
            }
        }

        ImGui::PopStyleVar();

        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(m_ContentOriginScreen.X, m_ContentOriginScreen.Y, m_Size.X, m_Size.Y);

        if (auto selection = m_Editor->GetSceneTree().GetSelection(); selection.size() == 1) {
            for (auto const& [id, entity] : selection) {

                auto m = entity->Transform.GetMatrix();
                if (ImGuizmo::Manipulate(
                    glm::value_ptr(m_Editor->GetCamera().GetView()),
                    glm::value_ptr(m_Editor->GetCamera().GetPerspective()),
                    GizmoModeToImGuizmo(m_GizmoMode),
                    GizmoSpaceToImGuizmo(m_GizmoSpace),
                    glm::value_ptr(m))) {
                    ImGuizmo::DecomposeMatrixToComponents(
                        glm::value_ptr(m),
                        entity->Transform.Position.Raw,
                        entity->Transform.EulerAngles.Raw,
                        entity->Transform.Scale.Raw);
                }
            }
        }
    }
    ImGui::End();
}

void ViewportWindow::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);

    dispatcher.Dispatch<OnKeyPressed>([this](OnKeyPressed const& e) -> bool {
        if (m_IsFocused) {
            if (e.Key == KeyboardKey::One) {
                m_GizmoMode = GizmoMode::Translation;
            }
            if (e.Key == KeyboardKey::Two) {
                m_GizmoMode = GizmoMode::Rotation;
            }
            if (e.Key == KeyboardKey::Three) {
                m_GizmoMode = GizmoMode::Scale;
            }
            if (e.Key == KeyboardKey::T) {}
        }
        return false;
    });
}
