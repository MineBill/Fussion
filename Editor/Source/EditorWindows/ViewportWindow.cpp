#include "EditorPCH.h"
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
#include "Fussion/Core/Time.h"
#include "Fussion/Events/KeyboardEvents.h"
#include "Fussion/Input/Input.h"
#include "Fussion/Math/Rect.h"

#include <tracy/Tracy.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <magic_enum/magic_enum.hpp>

using namespace Fussion;

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

void ViewportWindow::RenderStats() const
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    static int location = 1;
    if (location >= 0) {
        float const PAD = 10.0f;
        ImGuiViewport const* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = m_ContentOriginScreen;
        ImVec2 work_size = m_Size;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoMove;
    } else if (location == -2) {
        // Center window
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        window_flags |= ImGuiWindowFlags_NoMove;
    }

    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Stats Overlay", nullptr, window_flags)) {
        auto delta = Time::DeltaTime();
        ImGui::Text("CPU Time: %4.2fms", Time::SmoothDeltaTime() * 1000.0f);
        // ImGui::Text("Draw Calls: %d", Fussion::RHI::Device::Instance()->GetRenderStats().DrawCalls);
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Custom", nullptr, location == -1))
                location = -1;
            if (ImGui::MenuItem("Center", nullptr, location == -2))
                location = -2;
            if (ImGui::MenuItem("Top-left", nullptr, location == 0))
                location = 0;
            if (ImGui::MenuItem("Top-right", nullptr, location == 1))
                location = 1;
            if (ImGui::MenuItem("Bottom-left", nullptr, location == 2))
                location = 2;
            if (ImGui::MenuItem("Bottom-right", nullptr, location == 3))
                location = 3;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void ViewportWindow::OnDraw()
{
    ZoneScoped;

    // This is used to track whether the gizmo was draw the previous frame or not.

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(0, 0));
    bool opened = ImGui::Begin("Viewport");
    ImGui::PopStyleVar();

    if (opened) {
        static auto draw_gizmo = false;

        m_IsFocused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
        m_ContentOriginScreen = ImGui::GetCursorScreenPos();

        if (m_Editor->GetPlayState() == Editor::PlayState::Editing) {
            // TODO: If the ImGui viewports feature is disabled (like in linux), then the mouse position reported is wrong.
            if (m_IsFocused && Input::IsMouseButtonPressed(MouseButton::Left) && !(draw_gizmo && ImGuizmo::IsOver())) {
                auto mouse = Input::GetMousePosition() - (m_ContentOriginScreen - Application::Instance()->GetWindow().GetPosition());
                if (Rect::FromSize(m_Size).Contains(mouse)) {
                    // if (auto color = m_Editor->GetSceneRenderer().GetObjectPickingFrameBuffer()->ReadPixel(mouse); color.IsValue()) {
                    //     if (auto id = color.Value()[0]; id != 0) {
                    //         auto entity = Editor::GetActiveScene()->GetEntityFromLocalID(id);
                    //         Editor::GetSceneTree().SelectEntity(entity->GetHandle(), Input::IsKeyUp(Keys::LeftShift));
                    //     } else {
                    //         Editor::GetSceneTree().ClearSelection();
                    //     }
                    // }
                }
            }
        }

        if (auto size = ImGui::GetContentRegionAvail(); m_Size != Vector2(size)) {
            Editor::OnViewportResized(size);
            m_Size = size;
        }

        Vector2 origin = ImGui::GetCursorPos();

        {
            ZoneScopedN("Get image from set");
            auto image = Editor::Inst().GetSceneRenderer().GetRenderTarget();

            ImGui::Image(image.View, m_Size);
        }

        if (Editor::GetActiveScene() == nullptr) {
            ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
            constexpr ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoDocking |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav;

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

                auto metadata = Project::GetAssetManager()->GetMetadata(*handle);
                if (metadata.Type == AssetType::Scene) {
                    if (ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                        auto scene = AssetManager::GetAsset<Scene>(*handle);
                        scene.WaitUntilLoaded();

                        Editor::ChangeScene(scene);
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }

        RenderStats();

        ImGui::SetCursorPos(origin + Vector2(5, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);

        EUI::Button("Settings", [&] {

            ImGui::OpenPopup("EditorCameraSettings");
        }, { .Style = ButtonStyleViewportButton });

        EUI::Popup("EditorCameraSettings", [&] {
            if (ImGui::BeginMenu("Camera")) {
                EUI::Property("Speed", &Editor::GetCamera().Speed);
                EUI::Property("Near", &Editor::GetCamera().Near, EUI::PropTypeRange{ .Min = 0.0f, .Max = 100.0f });
                EUI::Property("Far", &Editor::GetCamera().Far, EUI::PropTypeRange{ .Min = 0.0f, .Max = 1000.0f });
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::BeginMenu("Views")) {
                    constexpr auto flags = ImGuiSelectableFlags_DontClosePopups;
                    auto& scene_debug_options = m_Editor->GetSceneRenderer().SceneDebugOptions.Data;
                    if (ImGui::Selectable("Show Cascade Boxes", scene_debug_options.ShowCascadeBoxes, flags)) {
                        scene_debug_options.ShowCascadeBoxes = !scene_debug_options.ShowCascadeBoxes;
                    }
                    if (ImGui::Selectable("Show Cascade Colors", scene_debug_options.ShowCascadeColors, flags)) {
                        scene_debug_options.ShowCascadeColors = !scene_debug_options.ShowCascadeColors;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Drawing")) {
                    auto& ctx = Editor::Inst().DebugDrawContext;
                    auto flags = ImGuiSelectableFlags_DontClosePopups;
                    for (auto const& [value, name] : magic_enum::enum_entries<DebugDrawFlag>()) {
                        if (ImGui::Selectable(name.data(), ctx.Flags.Test(value), flags)) {
                            ctx.Flags.Toggle(value);
                            LOG_DEBUGF("Flags: {}", ctx.Flags.value);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

        });

        ImGui::SameLine();
        ImGui::Dummy({ 10, 0 });
        ImGui::SameLine();

        fmt::memory_buffer out{};
        std::format_to(std::back_inserter(out), "Gizmo Mode: {}", magic_enum::enum_name(m_GizmoMode));
        std::string_view fuck{ out.data(), out.size() };

        EUI::Button(fuck.data(), [&] {
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

        out.clear();
        std::format_to(std::back_inserter(out), "Gizmo Mode: {}", magic_enum::enum_name(m_GizmoMode));
        fuck = { out.data(), out.size() };

        EUI::Button(fuck.data(), [&] {
            if (m_GizmoSpace == GizmoSpace::Local) {
                m_GizmoSpace = GizmoSpace::World;
            } else if (m_GizmoSpace == GizmoSpace::World) {
                m_GizmoSpace = GizmoSpace::Local;
            }
        }, { .Style = ButtonStyleViewportButton });

        ImGui::PopStyleVar();

        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(m_ContentOriginScreen.X, m_ContentOriginScreen.Y, m_Size.X, m_Size.Y);

        draw_gizmo = false;
        if (auto& selection = Editor::GetSceneTree().GetSelection(); selection.size() == 1 && m_Editor->GetPlayState() == Editor::PlayState::Editing) {
            static bool gizmo_activated = false;
            draw_gizmo = true;
            for (auto const& id : selection | std::views::keys) {
                auto const& entity = Editor::GetActiveScene()->GetEntity(id);

                auto m = entity->Transform.GetMatrix();
                if (ImGuizmo::Manipulate(
                    glm::value_ptr(Editor::GetCamera().GetView()),
                    glm::value_ptr(Editor::GetCamera().GetPerspective()),
                    GizmoModeToImGuizmo(m_GizmoMode),
                    GizmoSpaceToImGuizmo(m_GizmoSpace),
                    glm::value_ptr(m))) {
                    ImGuizmo::DecomposeMatrixToComponents(
                        glm::value_ptr(m),
                        entity->Transform.Position.Raw,
                        entity->Transform.EulerAngles.Raw,
                        entity->Transform.Scale.Raw);
                    if (!gizmo_activated) {
                        gizmo_activated = true;
                        switch (m_GizmoMode) {
                        case GizmoMode::Translation:
                            m_Editor->Undo.PushSingle(&entity->Transform.Position, "Gizmo LocalPosition");
                        case GizmoMode::Rotation:
                            m_Editor->Undo.PushSingle(&entity->Transform.EulerAngles, "Gizmo LocalEulerAngles");
                        case GizmoMode::Scale:
                            m_Editor->Undo.PushSingle(&entity->Transform.Scale, "Gizmo LocalScale");
                        }
                    }

                    Editor::GetActiveScene()->SetDirty();
                } else {
                    if (gizmo_activated && !ImGuizmo::IsUsingAny()) {
                        gizmo_activated = false;
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
