#include "FussionPCH.h"
#include "Camera.h"

#include "Core/Application.h"
#include "Fussion/Math/Math.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"

namespace Fussion {
#if FSN_DEBUG_DRAW
    void Camera::on_debug_draw(DebugDrawContext& ctx)
    {
        (void)ctx;
        auto aspect = Application::inst()->window().size().aspect();
        m_perspective = glm::perspective(glm::radians(fov), aspect, near, far);
        auto corners = Math::get_frustum_corners_world_space(m_perspective, m_owner->transform.camera_matrix());

        constexpr auto color = Color::SkyBlue;

        Debug::draw_line(corners[0], corners[1], 0.0, color);
        Debug::draw_line(corners[2], corners[3], 0.0, color);
        Debug::draw_line(corners[4], corners[5], 0.0, color);
        Debug::draw_line(corners[6], corners[7], 0.0, color);

        Debug::draw_line(corners[0], corners[2], 0.0, color);
        Debug::draw_line(corners[2], corners[6], 0.0, color);
        Debug::draw_line(corners[6], corners[4], 0.0, color);
        Debug::draw_line(corners[4], corners[0], 0.0, color);

        Debug::draw_line(corners[1], corners[3], 0.0, color);
        Debug::draw_line(corners[3], corners[7], 0.0, color);
        Debug::draw_line(corners[7], corners[5], 0.0, color);
        Debug::draw_line(corners[5], corners[1], 0.0, color);
    }
#endif

    void Camera::on_update(f32 delta)
    {
        (void)delta;

        auto aspect = Application::inst()->window().size().aspect();
        m_perspective = glm::perspective(glm::radians(fov), aspect, near, far);
    }

    Ref<Component> Camera::clone()
    {
        auto camera = make_ref<Camera>();
        camera->far = far;
        camera->near = near;
        camera->fov = fov;
        return camera;
    }

    void Camera::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(far);
        FSN_SERIALIZE_MEMBER(near);
        FSN_SERIALIZE_MEMBER(fov);
    }

    void Camera::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(far);
        FSN_DESERIALIZE_MEMBER(near);
        FSN_DESERIALIZE_MEMBER(fov);
    }
}
