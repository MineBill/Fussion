#include "ViewportWindow.h"
#include "Layers/Editor.h"
#include "EditorApplication.h"
#include "SceneRenderer.h"
#include "Layers/ImGuiLayer.h"
#include "Fussion/Assets/AssetManager.h"
#include "EditorUI.h"

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

        if (m_Editor->GetActiveScene() == nullptr) {
            ImGui::SetNextWindowPos(m_ContentOriginScreen + m_Size / 2.f + Vector2(0, m_Size.Y * 0.2f), 0, Vector2(0.5, 0.5));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(20, 20));
            defer(ImGui::PopStyleVar());

            ImGui::SetNextWindowBgAlpha(0.35f);
            if (ImGui::Begin("No Scene Warning", nullptr, window_flags)) {
                ImGui::PushFont(EditorStyle::GetStyle().Fonts[EditorFont::RegularHuge]);
                defer(ImGui::PopFont());

                ImGui::TextUnformatted("No scene loaded!");
            }
            ImGui::End();
        }

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
                }
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::SetCursorPos(origin + Vector2(5, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);

        EUI::Button("Camera", [&] {
            ImGui::OpenPopup("EditorCameraSettings");
        }, { .Style = ButtonStyleViewportButton });

        EUI::Popup("EditorCameraSettings", [&] {
            EUI::Property("Speed", &m_Editor->GetCamera().Speed);
        });

        ImGui::SameLine();
        ImGui::Dummy({ 10, 0 });
        ImGui::SameLine();

        EUI::Button(std::format("Gizmo Mode: {}", magic_enum::enum_name(m_GizmoMode)), [&] {
            ImGui::OpenPopup("GizmoSelection");
        }, { .Style = ButtonStyleViewportButton });

        EUI::Popup("GizmoSelection", [&] {
            if (ImGui::MenuItem("Translation", "1", m_GizmoMode == GizmoMode::Translation)) {
                m_GizmoMode = GizmoMode::Translation;
            }
            if (ImGui::MenuItem("Rotation", "2", m_GizmoMode == GizmoMode::Rotation)) {
                m_GizmoMode = GizmoMode::Rotation;
            }
            if (ImGui::MenuItem("Scale", "3", m_GizmoMode == GizmoMode::Scale)) {
                m_GizmoMode = GizmoMode::Scale;
            }
        });

        ImGui::SameLine();

        EUI::Button(std::format("Gizmo Space: {}", magic_enum::enum_name(m_GizmoSpace)), [&] {
            if (m_GizmoSpace == GizmoSpace::Local) {
                m_GizmoSpace = GizmoSpace::World;
            } else if (m_GizmoSpace == GizmoSpace::World) {
                m_GizmoSpace = GizmoSpace::Local;
            }
        }, { .Style = ButtonStyleViewportButton });

        ImGui::PopStyleVar();

        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(m_ContentOriginScreen.X, m_ContentOriginScreen.Y, m_Size.X, m_Size.Y);

        static bool activated = false;
        if (auto& selection = m_Editor->GetSceneTree().GetSelection(); selection.size() == 1) {
            for (auto const& id : selection | std::views::keys) {
                auto const& entity = m_Editor->GetActiveScene()->GetEntity(id);

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
                    if (!activated) {
                        activated = true;
                        switch (m_GizmoMode) {
                        case GizmoMode::Translation:
                            m_Editor->Undo.PushSingle(&entity->Transform.Position, "Gizmo LocalPosition");
                        case GizmoMode::Rotation:
                            m_Editor->Undo.PushSingle(&entity->Transform.EulerAngles, "Gizmo LocalEulerAngles");
                        case GizmoMode::Scale:
                            m_Editor->Undo.PushSingle(&entity->Transform.Scale, "Gizmo LocalScale");
                        }
                    }

                    m_Editor->GetActiveScene()->SetDirty();
                } else {
                    if (activated && !ImGuizmo::IsUsingAny()) {
                        activated = false;
                        switch (m_GizmoMode) {
                        case GizmoMode::Translation:
                            m_Editor->Undo.CommitTag("Gizmo LocalPosition");
                        case GizmoMode::Rotation:
                            m_Editor->Undo.CommitTag("Gizmo LocalEulerAngles");
                        case GizmoMode::Scale:
                            m_Editor->Undo.CommitTag("Gizmo LocalScale");
                        }
                    }
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
            if (e.Key == Keys::One) {
                m_GizmoMode = GizmoMode::Translation;
            }
            if (e.Key == Keys::Two) {
                m_GizmoMode = GizmoMode::Rotation;
            }
            if (e.Key == Keys::Three) {
                m_GizmoMode = GizmoMode::Scale;
            }
            if (e.Key == Keys::T) {}
        }
        return false;
    });
}
