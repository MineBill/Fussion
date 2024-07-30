#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Events/Event.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector3.h"

class EditorCamera {
public:
    Vector3 Position{};
    f32 Speed{ 1.0f };
    f32 Near{ 0.1f }, Far{ 1000.0f };

    void OnUpdate(f32);
    void HandleEvent(Fussion::Event& event);

    void Resize(Vector2 new_size);

    void SetFocus(bool focused);

    auto GetPerspective() const -> Mat4 const& { return m_Perspective; }
    auto GetView() const -> Mat4 const& { return m_View; }
    auto GetDirection() const -> Vector3 { return m_Direction; };

private:
    Mat4 m_Perspective{ 1.0f }, m_View{ 1.0f };

    Vector2 m_ScreenSize{};
    Vector3 m_EulerAngles{};
    Vector3 m_Direction{};
    f32 m_FOV{ 50.0f };
    bool m_HasFocus{ false };
    bool m_HadFocus{ false };
    bool m_CapturedMouse{ false };
};
