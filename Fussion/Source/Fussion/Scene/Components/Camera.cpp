#include "FussionPCH.h"
#include "Camera.h"

#include "Core/Application.h"
#include "Scene/Entity.h"
#include "Fussion/Math/Math.h"

namespace Fussion {
#if FSN_DEBUG_DRAW
    void Camera::OnDebugDraw(DebugDrawContext& ctx)
    {
        (void)ctx;
        auto aspect = Application::Instance()->GetWindow().GetSize().Aspect();
        m_Perspective = glm::perspective(glm::radians(Fov), aspect, Near, Far);
        auto corners = Math::GetFrustumCornersWorldSpace(m_Perspective, m_Owner->Transform.GetCameraMatrix());

        constexpr auto color = Color::SkyBlue;

        Debug::DrawLine(corners[0], corners[1], 0.0, color);
        Debug::DrawLine(corners[2], corners[3], 0.0, color);
        Debug::DrawLine(corners[4], corners[5], 0.0, color);
        Debug::DrawLine(corners[6], corners[7], 0.0, color);

        Debug::DrawLine(corners[0], corners[2], 0.0, color);
        Debug::DrawLine(corners[2], corners[6], 0.0, color);
        Debug::DrawLine(corners[6], corners[4], 0.0, color);
        Debug::DrawLine(corners[4], corners[0], 0.0, color);

        Debug::DrawLine(corners[1], corners[3], 0.0, color);
        Debug::DrawLine(corners[3], corners[7], 0.0, color);
        Debug::DrawLine(corners[7], corners[5], 0.0, color);
        Debug::DrawLine(corners[5], corners[1], 0.0, color);
    }
#endif

    void Camera::OnUpdate(f32 delta)
    {
        (void)delta;

        auto aspect = Application::Instance()->GetWindow().GetSize().Aspect();
        m_Perspective = glm::perspective(glm::radians(Fov), aspect, Near, Far);
    }
}
