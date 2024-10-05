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
        // This needs to be here because the update doesn't run during edit mode.
        auto aspect = Application::Self()->GetWindow().Size().Aspect();
        m_Perspective = glm::perspective(glm::radians(fov), aspect, near, far);
        auto corners = Math::GetFrustumCornersWorldSpace(m_Perspective, m_Owner->Transform.AsCameraMatrix());

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

        auto aspect = Application::Self()->GetWindow().Size().Aspect();
        m_Perspective = glm::perspective(glm::radians(fov), aspect, near, far);
    }

    Ref<Component> Camera::Clone()
    {
        auto camera = MakeRef<Camera>();
        camera->far = far;
        camera->near = near;
        camera->fov = fov;
        return camera;
    }

    void Camera::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(far);
        FSN_SERIALIZE_MEMBER(near);
        FSN_SERIALIZE_MEMBER(fov);
    }

    void Camera::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(far);
        FSN_DESERIALIZE_MEMBER(near);
        FSN_DESERIALIZE_MEMBER(fov);
    }
}
