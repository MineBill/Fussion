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
#include "Fussion/Math/Math.h"
#include "Fussion/Math/Vector4.h"

using namespace Fussion;

void EditorCamera::OnUpdate(const f32 delta)
{
    auto rotation = glm::eulerAngleZXY(
        glm::radians(m_EulerAngles.Z),
        glm::radians(m_EulerAngles.X),
        glm::radians(m_EulerAngles.Y));

    if (m_CapturedMouse) {
        auto const x = Input::GetAxis(Keys::D, Keys::A);
        auto const y = Input::GetAxis(Keys::Space, Keys::LeftControl);
        auto const z = Input::GetAxis(Keys::S, Keys::W);
        auto input = Vector3(x, y, z);

        Position += Vector3(Vector4(input, 0.0f) * rotation) * delta * Speed;
    }

    m_Direction = glm::mat3(rotation) * Vector3::Forward;

    m_Perspective = glm::perspective(glm::radians(m_FOV), m_ScreenSize.X / m_ScreenSize.Y, Near, Far);

    m_View = rotation * glm::inverse(glm::translate(Mat4(1.0), CAST(glm::vec3, Position)));
}

void EditorCamera::HandleEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseMoved>([this](MouseMoved const& mouse_moved) -> bool {
        if (m_CapturedMouse) {
            const auto input = Vector3(mouse_moved.RelY, mouse_moved.RelX, 0);
            m_EulerAngles += input * 0.1f;
        }
        return false;
    });

    dispatcher.Dispatch<MouseWheelMoved>([this](MouseWheelMoved const& mouse_wheel_moved) -> bool {
        if (m_CapturedMouse) {
            constexpr auto max_speed = 10.0f;
            constexpr auto min_speed = 0.1f;

            Speed += mouse_wheel_moved.Y * (Speed / max_speed);
            Speed = Math::Clamp(Speed, min_speed, max_speed);
        }
        return false;
    });

    dispatcher.Dispatch<MouseButtonPressed>([this](MouseButtonPressed const& mouse_pressed) -> bool {
        if (m_HasFocus && mouse_pressed.Button == MouseButton::Right) {
            m_CapturedMouse = true;
            Application::Instance()->GetWindow().SetMouseMode(MouseMode::Locked);
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }
        return false;
    });

    dispatcher.Dispatch<MouseButtonReleased>([this](MouseButtonReleased const& mouse_released) -> bool {
        if (mouse_released.Button == MouseButton::Right && m_CapturedMouse) {
            m_CapturedMouse = false;
            Application::Instance()->GetWindow().SetMouseMode(MouseMode::Unlocked);
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }
        return false;
    });
}

void EditorCamera::Resize(Vector2 new_size)
{
    m_ScreenSize = new_size;
}

void EditorCamera::SetFocus(const bool focused)
{
    m_HasFocus = focused;
}
