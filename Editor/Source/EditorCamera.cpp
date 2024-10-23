#include "EditorPCH.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "EditorCamera.h"
#include "Fussion/Core/Application.h"
#include "Fussion/Core/Time.h"
#include "Fussion/Events/KeyboardEvents.h"
#include "Fussion/Input/Input.h"
#include "Fussion/Math/Math.h"
#include "Fussion/Math/Vector4.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

using namespace Fussion;

void EditorCamera::OnUpdate(f32 delta)
{
    (void)delta;
    auto rotation = glm::eulerAngleZXY(
        glm::radians(EulerAngles.z),
        glm::radians(EulerAngles.x),
        glm::radians(EulerAngles.y));

    auto fake_rot = glm::eulerAngleYXZ(
        glm::radians(EulerAngles.y),
        glm::radians(EulerAngles.x),
        glm::radians(EulerAngles.z));
    m_Direction = glm::mat3(fake_rot) * -Vector3::Forward;
    m_Direction = Vector3();

    if (m_CapturedMouse) {
        auto const x = Input::GetAxis(Keys::D, Keys::A);
        auto const y = Input::GetAxis(Keys::Space, Keys::LeftControl);
        auto const z = Input::GetAxis(Keys::S, Keys::W);
        auto input = Vector3(x, y, z);

        Position += Vector3(Vector4(input, 0.0f) * rotation) * Time::SmoothDeltaTime() * Speed;
    }

    m_Perspective = glm::perspective(glm::radians(Fov), m_ScreenSize.x / m_ScreenSize.y, Near, Far);

    m_ViewRotation = rotation;
    m_View = m_ViewRotation * glm::inverse(glm::translate(Mat4(1.0), CAST(glm::vec3, Position)));
}

void EditorCamera::HandleEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseMoved>([this](MouseMoved const& mouse_moved) -> bool {
        if (m_CapturedMouse) {
            auto input = Vector3(mouse_moved.rel_y, mouse_moved.rel_x, 0);
            EulerAngles += input * 0.1f;
        }
        return false;
    });

    dispatcher.Dispatch<MouseWheelMoved>([this](MouseWheelMoved const& mouse_wheel_moved) -> bool {
        if (m_CapturedMouse) {
            constexpr auto max_speed = 10.0f;
            constexpr auto min_speed = 0.1f;

            Speed += mouse_wheel_moved.y * (Speed / max_speed);
            Speed = Math::Clamp(Speed, min_speed, max_speed);
        }
        return false;
    });

    dispatcher.Dispatch<MouseButtonPressed>([this](MouseButtonPressed const& mouse_pressed) -> bool {
        if (m_HasFocus && mouse_pressed.button == MouseButton::Right) {
            m_CapturedMouse = true;
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
            Application::Self()->GetWindow().SetMouseMode(MouseMode::Locked);
        }
        return false;
    });

    dispatcher.Dispatch<MouseButtonReleased>([this](MouseButtonReleased const& mouse_released) -> bool {
        if (mouse_released.button == MouseButton::Right && m_CapturedMouse) {
            m_CapturedMouse = false;
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            Application::Self()->GetWindow().SetMouseMode(MouseMode::Unlocked);
        }
        return false;
    });
}

void EditorCamera::Resize(Vector2 const& new_size)
{
    m_ScreenSize = new_size;
}

void EditorCamera::SetFocus(bool focused)
{
    m_HasFocus = focused;
}

auto EditorCamera::ToRenderCamera() const -> RenderCamera
{
    return RenderCamera {
        .perspective = m_Perspective,
        .view = m_View,
        .position = Position,
        .near = Near,
        .far = Far,
        .direction = Direction(),
    };
}
