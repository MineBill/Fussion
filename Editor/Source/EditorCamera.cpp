#include "EditorPCH.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "EditorCamera.h"

#include <imgui.h>

#include "Fussion/Events/KeyboardEvents.h"
#include "Fussion/Input/Input.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Fussion/Core/Application.h"
#include "Fussion/Core/Time.h"
#include "Fussion/Math/Math.h"
#include "Fussion/Math/Vector4.h"

using namespace Fussion;

void EditorCamera::on_update(f32 delta)
{
    (void)delta;
    auto rotation = glm::eulerAngleZXY(
        glm::radians(euler_angles.z),
        glm::radians(euler_angles.x),
        glm::radians(euler_angles.y));

    auto fake_rot = glm::eulerAngleYXZ(
        glm::radians(euler_angles.y),
        glm::radians(euler_angles.x),
        glm::radians(euler_angles.z));
    m_direction = glm::mat3(fake_rot) * -Vector3::Forward;
    m_direction = Vector3();

    if (m_captured_mouse) {
        auto const x = Input::get_axis(Keys::D, Keys::A);
        auto const y = Input::get_axis(Keys::Space, Keys::LeftControl);
        auto const z = Input::get_axis(Keys::S, Keys::W);
        auto input = Vector3(x, y, z);

        position += Vector3(Vector4(input, 0.0f) * rotation) * Time::smooth_delta_time() * speed;
    }

    m_perspective = glm::perspective(glm::radians(fov), m_screen_size.x / m_screen_size.y, near, far);

    m_view = rotation * glm::inverse(glm::translate(Mat4(1.0), CAST(glm::vec3, position)));
}

void EditorCamera::handle_event(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<MouseMoved>([this](MouseMoved const& mouse_moved) -> bool {
        if (m_captured_mouse) {
            auto input = Vector3(mouse_moved.rel_y, mouse_moved.rel_x, 0);
            euler_angles += input * 0.1f;
        }
        return false;
    });

    dispatcher.dispatch<MouseWheelMoved>([this](MouseWheelMoved const& mouse_wheel_moved) -> bool {
        if (m_captured_mouse) {
            constexpr auto max_speed = 10.0f;
            constexpr auto min_speed = 0.1f;

            speed += mouse_wheel_moved.Y * (speed / max_speed);
            speed = Math::clamp(speed, min_speed, max_speed);
        }
        return false;
    });

    dispatcher.dispatch<MouseButtonPressed>([this](MouseButtonPressed const& mouse_pressed) -> bool {
        if (m_has_focus && mouse_pressed.button == MouseButton::Right) {
            m_captured_mouse = true;
            Application::inst()->window().set_mouse_mode(MouseMode::Locked);
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }
        return false;
    });

    dispatcher.dispatch<MouseButtonReleased>([this](MouseButtonReleased const& mouse_released) -> bool {
        if (mouse_released.button == MouseButton::Right && m_captured_mouse) {
            m_captured_mouse = false;
            Application::inst()->window().set_mouse_mode(MouseMode::Unlocked);
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }
        return false;
    });
}

void EditorCamera::resize(Vector2 const& new_size)
{
    m_screen_size = new_size;
}

void EditorCamera::set_focus(bool focused)
{
    m_has_focus = focused;
}

auto EditorCamera::to_render_camera() const -> RenderCamera
{
    return RenderCamera{
        .perspective = m_perspective,
        .view = m_view,
        .position = position,
        .near = near,
        .far = far,
        .direction = direction(),
    };
}
