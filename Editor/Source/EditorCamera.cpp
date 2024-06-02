#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "EditorCamera.h"

#include <imgui.h>

#include "Engin5/Events/KeyboardEvents.h"
#include "Engin5/Input/Input.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Engin5/Log/Formatters.h"
#include "Engin5/Core/Application.h"

using namespace Engin5;

void EditorCamera::OnUpdate(const f32 delta)
{
    auto rotation = glm::eulerAngleZXY(
        glm::radians(m_EulerAngles.z),
        glm::radians(m_EulerAngles.x),
        glm::radians(m_EulerAngles.y));

    if (m_CapturedMouse) {
        const auto x = Input::GetAxis(KeyboardKey::D, KeyboardKey::A);
        const auto y = Input::GetAxis(KeyboardKey::Space, KeyboardKey::LeftControl);
        const auto z = Input::GetAxis(KeyboardKey::S, KeyboardKey::W);
        auto input = Vector3(x, y, z);

        // auto forward = glm::quat(m_EulerAngles) * Vector3(0, 0, -1);
        // input *= forward;
        m_Position += Vector3(Vector4(input, 0.0f) * rotation) * delta * 0.001f;
    }

    m_Perspective = glm::perspective(glm::radians(m_FOV), m_ScreenSize.x / m_ScreenSize.y, 0.1f, 1000.0f);

    // auto rotation = glm::rotate(Mat4(1.0), glm::radians(m_EulerAngles.z), Vector3(0, 0, 1));
    // rotation = glm::rotate(rotation, glm::radians(m_EulerAngles.x), Vector3(1, 0, 0));
    // rotation = glm::rotate(rotation, glm::radians(m_EulerAngles.y), Vector3(0, 1, 0));

    m_View = rotation * glm::inverse(glm::translate(Mat4(1.0), m_Position));
}

void EditorCamera::HandleEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<MouseMoved>([this](MouseMoved& mouse_moved) -> bool {
        if (m_CapturedMouse) {
            const auto input = Vector3(mouse_moved.RelY, mouse_moved.RelX, 0);
            LOG_WARNF("Big mouse delta detected: {}", input);
            m_EulerAngles += input * 0.1f;
        }
        return false;
    });

    dispatcher.Dispatch<MouseButtonPressed>([this](MouseButtonPressed const& mouse_pressed) -> bool {
        if (m_HasFocus && mouse_pressed.Button == MouseButton::Right) {
            m_CapturedMouse = true;
            Application::Instance()->GetWindow()->SetMouseMode(MouseMode::Locked);
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }
        return false;
    });

    dispatcher.Dispatch<MouseButtonReleased>([this](MouseButtonReleased const& mouse_released) -> bool {
        if (mouse_released.Button == MouseButton::Right && m_CapturedMouse) {
            m_CapturedMouse = false;
            Application::Instance()->GetWindow()->SetMouseMode(MouseMode::Unlocked);
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