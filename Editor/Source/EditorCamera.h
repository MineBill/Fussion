#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Events/Event.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector3.h"
#include "SceneRenderer.h"

class EditorCamera {
public:
    Vector3 Position {};
    Vector3 EulerAngles {};
    f32 Speed { 1.0f };
    f32 Near { 0.1f }, Far { 1000.0f };
    f32 Fov { 60.0f };

    void OnUpdate(f32);
    void HandleEvent(Fussion::Event& event);

    void Resize(Vector2 const& new_size);

    void SetFocus(bool focused);

    auto Perspective() const -> Mat4 const& { return m_Perspective; }
    auto View() const -> Mat4 const& { return m_View; }
    auto RotationMatrix() const -> Mat4 const& { return m_ViewRotation; }
    auto Direction() const -> Vector3 { return m_Direction; }

    auto ToRenderCamera() const -> RenderCamera;

private:
    Mat4 m_Perspective { 1.0f }, m_View { 1.0f }, m_ViewRotation { 1.0f };

    Vector2 m_ScreenSize {};
    Vector3 m_Direction {};
    bool m_HasFocus { false };
    bool m_CapturedMouse { false };
};
