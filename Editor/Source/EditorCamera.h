#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Events/Event.h"

class EditorCamera
{
public:
    void OnUpdate(f32);
    void HandleEvent(Fussion::Event& event);

    void Resize(Vector2 new_size);

    void SetFocus(bool focused);

    Mat4 const& GetPerspective() const { return m_Perspective; }
    Mat4 const& GetView() const { return m_View; }
    Vector3 const& GetPosition() const { return m_Position; }

private:
    Mat4 m_Perspective{1.0f}, m_View{1.0f};

    Vector2 m_ScreenSize{};
    Vector3 m_Position{};
    Vector3 m_EulerAngles{};
    f32 m_FOV{50.0f};
    bool m_HasFocus{false};
    bool m_HadFocus{false};
    bool m_CapturedMouse{false};
};
