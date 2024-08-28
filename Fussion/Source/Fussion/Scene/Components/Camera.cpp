#include "FussionPCH.h"
#include "Camera.h"

#include "Core/Application.h"
#include "Fussion/Math/Math.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"

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

    Ref<Component> Camera::Clone()
    {
        auto camera = MakeRef<Camera>();
        camera->Far = Far;
        camera->Near = Near;
        camera->Fov = Fov;
        return camera;
    }

    void Camera::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(Far);
        FSN_SERIALIZE_MEMBER(Near);
        FSN_SERIALIZE_MEMBER(Fov);
    }

    void Camera::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(Far);
        FSN_DESERIALIZE_MEMBER(Near);
        FSN_DESERIALIZE_MEMBER(Fov);
    }
}
