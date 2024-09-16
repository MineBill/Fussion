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
#include "Fussion/Assets/Model.h"
#include "Fussion/Core/Time.h"
#include "Fussion/Events/KeyboardEvents.h"
#include "Fussion/Input/Input.h"
#include "Fussion/Math/Rect.h"
#include "Fussion/Scene/Components/MeshRenderer.h"

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

void ViewportWindow::render_stats() const
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    static int location = 1;
    if (location >= 0) {
        float const PAD = 10.0f;
        ImGuiViewport const* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = m_content_origin_screen;
        ImVec2 work_size = m_size;
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
        ImGui::Text("CPU Time: %4.2fms", Time::smooth_delta_time() * 1000.0f);
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

void ViewportWindow::on_draw()
{
    ZoneScoped;

    // This is used to track whether the gizmo was draw the previous frame or not.

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(0, 0));
    bool opened = ImGui::Begin("Viewport");
    ImGui::PopStyleVar();

    if (opened) {
        static auto draw_gizmo = false;

        m_is_focused = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();
        m_content_origin_screen = ImGui::GetCursorScreenPos();

        if (m_editor->play_state() == Editor::PlayState::Editing) {
            // TODO: If the ImGui viewports feature is disabled (like in linux), then the mouse position reported is wrong.
            if (m_is_focused && Input::is_mouse_button_pressed(MouseButton::Left) && !(draw_gizmo && ImGuizmo::IsOver())) {
                auto mouse = Input::mouse_position() - (m_content_origin_screen - Application::inst()->window().position());
                if (Rect::from_size(m_size).contains(mouse)) {
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

        if (auto size = ImGui::GetContentRegionAvail(); m_size != Vector2(size)) {
            Editor::on_viewport_resized(size);
            m_size = size;
        }

        Vector2 origin = ImGui::GetCursorPos();

        {
            ZoneScopedN("Get image from set");
            GPU::Texture image;
            switch (m_texture_view_mode) {
            case TEXTURE_SCENE:
                image = Editor::inst().scene_renderer().render_target();
                break;
            case TEXTURE_GBUFFER_POSITION:
                image = Editor::inst().scene_renderer().m_gbuffer.rt_position;
                break;
            case TEXTURE_GBUFFER_NORMAL:
                image = Editor::inst().scene_renderer().m_gbuffer.rt_normal;
                break;
            case TEXTURE_SSAO:
                image = Editor::inst().scene_renderer().ssao_blur.render_target();
                break;
            }

            ImGui::Image(image.view, m_size);
        }

        if (Editor::active_scene() == nullptr) {
            ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
            constexpr ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoDocking |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav;

            ImGui::SetNextWindowPos(m_content_origin_screen + m_size / 2.f + Vector2(0, m_size.y * 0.2f), 0, Vector2(0.5, 0.5));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(20, 20));
            defer(ImGui::PopStyleVar());

            ImGui::SetNextWindowBgAlpha(0.35f);
            if (ImGui::Begin("No Scene Warning", nullptr, window_flags)) {
                ImGui::PushFont(EditorStyle::get_style().fonts[EditorFont::RegularHuge]);
                defer(ImGui::PopFont());

                ImGui::TextUnformatted("No scene loaded!");
            }
            ImGui::End();
        }

        if (ImGui::BeginDragDropTarget()) {
            auto* payload = ImGui::GetDragDropPayload();
            if (strcmp(payload->DataType, "CONTENT_BROWSER_ASSET") == 0) {
                auto handle = CAST(AssetHandle*, payload->Data);

                auto metadata = Project::asset_manager()->get_metadata(*handle);
                if (metadata.type == AssetType::Scene) {
                    if (ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
                        auto scene = AssetManager::get_asset<Scene>(*handle);
                        scene.wait_until_loaded();

                        Editor::change_scene(scene);
                    }
                } else if (metadata.type == AssetType::Model) {
                    if (ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET") && m_editor->active_scene() != nullptr) {
                        auto model = AssetManager::get_asset<Model>(*handle);
                        auto entity = m_editor->active_scene()->create_entity(metadata.name);
                        auto mr = entity->add_component<MeshRenderer>();
                        mr->model = model;
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }

        render_stats();

        ImGui::SetCursorPos(origin + Vector2(5, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);

        EUI::button("Settings", [&] {

            ImGui::OpenPopup("EditorCameraSettings");
        }, { .style = ButtonStyleViewportButton });

        EUI::popup("EditorCameraSettings", [&] {
            if (ImGui::BeginMenu("Camera")) {
                EUI::property("Speed", &Editor::camera().speed);
                EUI::property("Near", &Editor::camera().near, EUI::PropTypeRange{ .min = 0.0f, .max = 100.0f });
                EUI::property("Far", &Editor::camera().far, EUI::PropTypeRange{ .min = 0.0f, .max = 1000.0f });
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::BeginMenu("Views")) {
                    constexpr auto flags = ImGuiSelectableFlags_DontClosePopups;
                    if (ImGui::BeginMenu("Scene")) {
                        if (ImGui::Selectable("Default", m_texture_view_mode == TEXTURE_SCENE, flags)) {
                            m_texture_view_mode = TEXTURE_SCENE;
                        }
                        auto& scene_debug_options = m_editor->scene_renderer().scene_debug_options.data;
                        if (ImGui::Selectable("Show Cascade Boxes", scene_debug_options.show_cascade_boxes, flags)) {
                            scene_debug_options.show_cascade_boxes = !scene_debug_options.show_cascade_boxes;
                        }
                        if (ImGui::Selectable("Show Cascade Colors", scene_debug_options.show_cascade_colors, flags)) {
                            scene_debug_options.show_cascade_colors = !scene_debug_options.show_cascade_colors;
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("G-Buffer")) {
                        if (ImGui::Selectable("Position", m_texture_view_mode == TEXTURE_GBUFFER_POSITION, flags)) {
                            m_texture_view_mode = TEXTURE_GBUFFER_POSITION;
                        }
                        if (ImGui::Selectable("Normal", m_texture_view_mode == TEXTURE_GBUFFER_NORMAL, flags)) {
                            m_texture_view_mode = TEXTURE_GBUFFER_NORMAL;
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::Selectable("SSAO", m_texture_view_mode == TEXTURE_SSAO, flags)) {
                        m_texture_view_mode = TEXTURE_SSAO;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Drawing")) {
                    auto& ctx = Editor::inst().debug_draw_context;
                    auto flags = ImGuiSelectableFlags_DontClosePopups;
                    for (auto const& [value, name] : magic_enum::enum_entries<DebugDrawFlag>()) {
                        if (ImGui::Selectable(name.data(), ctx.flags.test(value), flags)) {
                            ctx.flags.toggle(value);
                            LOG_DEBUGF("Flags: {}", ctx.flags.value);
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
        std::format_to(std::back_inserter(out), "Gizmo Mode: {}", magic_enum::enum_name(m_gizmo_mode));
        std::string_view fuck{ out.data(), out.size() };

        EUI::button(fuck.data(), [&] {
            ImGui::OpenPopup("GizmoSelection");
        }, { .style = ButtonStyleViewportButton });

        EUI::popup("GizmoSelection", [&] {
            if (ImGui::MenuItem("Translation", "1", m_gizmo_mode == GizmoMode::Translation)) {
                m_gizmo_mode = GizmoMode::Translation;
            }
            if (ImGui::MenuItem("Rotation", "2", m_gizmo_mode == GizmoMode::Rotation)) {
                m_gizmo_mode = GizmoMode::Rotation;
            }
            if (ImGui::MenuItem("Scale", "3", m_gizmo_mode == GizmoMode::Scale)) {
                m_gizmo_mode = GizmoMode::Scale;
            }
        });

        ImGui::SameLine();

        out.clear();
        std::format_to(std::back_inserter(out), "Gizmo Space: {}", magic_enum::enum_name(m_gizmo_space));
        fuck = { out.data(), out.size() };

        EUI::button(fuck.data(), [&] {
            if (m_gizmo_space == GizmoSpace::Local) {
                m_gizmo_space = GizmoSpace::World;
            } else if (m_gizmo_space == GizmoSpace::World) {
                m_gizmo_space = GizmoSpace::Local;
            }
        }, { .style = ButtonStyleViewportButton });

        ImGui::PopStyleVar();

        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(m_content_origin_screen.x, m_content_origin_screen.y, m_size.x, m_size.y);

        draw_gizmo = false;
        if (auto& selection = Editor::scene_tree().selection(); selection.size() == 1 && m_editor->play_state() == Editor::PlayState::Editing) {
            static bool gizmo_activated = false;
            draw_gizmo = true;
            for (auto const& id : selection | std::views::keys) {
                auto const& entity = Editor::active_scene()->get_entity(id);

                Vector3 snap{ 0.5, 0.5, 0.5 };

                auto m = entity->world_matrix();
                if (ImGuizmo::Manipulate(
                    glm::value_ptr(Editor::camera().view()),
                    glm::value_ptr(Editor::camera().perspective()),
                    GizmoModeToImGuizmo(m_gizmo_mode),
                    GizmoSpaceToImGuizmo(m_gizmo_space),
                    glm::value_ptr(m), nullptr, Input::is_key_down(Keys::LeftControl) ? snap.raw : nullptr)) {
                    ImGuizmo::DecomposeMatrixToComponents(
                        glm::value_ptr(m),
                        entity->transform.position.raw,
                        entity->transform.euler_angles.raw,
                        entity->transform.scale.raw);
                    if (!gizmo_activated) {
                        gizmo_activated = true;
                        switch (m_gizmo_mode) {
                        case GizmoMode::Translation:
                            m_editor->undo.push_single(&entity->transform.position, "Gizmo LocalPosition");
                        case GizmoMode::Rotation:
                            m_editor->undo.push_single(&entity->transform.euler_angles, "Gizmo LocalEulerAngles");
                        case GizmoMode::Scale:
                            m_editor->undo.push_single(&entity->transform.scale, "Gizmo LocalScale");
                        }
                    }

                    Editor::active_scene()->set_dirty();
                } else {
                    if (gizmo_activated && !ImGuizmo::IsUsingAny()) {
                        gizmo_activated = false;
                        switch (m_gizmo_mode) {
                        case GizmoMode::Translation:
                            m_editor->undo.commit_tag("Gizmo LocalPosition");
                        case GizmoMode::Rotation:
                            m_editor->undo.commit_tag("Gizmo LocalEulerAngles");
                        case GizmoMode::Scale:
                            m_editor->undo.commit_tag("Gizmo LocalScale");
                        }
                    }
                }
            }
        }
    }
    ImGui::End();
}

void ViewportWindow::on_event(Event& event)
{
    EventDispatcher dispatcher(event);

    dispatcher.dispatch<OnKeyPressed>([this](OnKeyPressed const& e) -> bool {
        if (m_is_focused) {
            if (e.key == Keys::One) {
                m_gizmo_mode = GizmoMode::Translation;
            }
            if (e.key == Keys::Two) {
                m_gizmo_mode = GizmoMode::Rotation;
            }
            if (e.key == Keys::Three) {
                m_gizmo_mode = GizmoMode::Scale;
            }
            if (e.key == Keys::T) {}
        }
        return false;
    });
}
