#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Events/Event.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector3.h"
#include "SceneRenderer.h"

class EditorCamera {
public:
    Vector3 position {};
    Vector3 euler_angles {};
    f32 speed { 1.0f };
    f32 near { 0.1f }, far { 1000.0f };
    f32 fov { 60.0f };

    void on_update(f32);
    void handle_event(Fussion::Event& event);

    void resize(Vector2 const& new_size);

    void set_focus(bool focused);

    auto perspective() const -> Mat4 const& { return m_perspective; }
    auto view() const -> Mat4 const& { return m_view; }
    auto rotation() const -> Mat4 const& { return m_view_rotation; }
    auto direction() const -> Vector3 { return m_direction; }

    auto to_render_camera() const -> RenderCamera;

private:
    Mat4 m_perspective { 1.0f }, m_view { 1.0f }, m_view_rotation { 1.0f };

    Vector2 m_screen_size {};
    Vector3 m_direction {};
    bool m_has_focus { false };
    bool m_captured_mouse { false };
};
