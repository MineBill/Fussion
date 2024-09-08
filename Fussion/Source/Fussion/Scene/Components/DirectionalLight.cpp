#include "FussionPCH.h"
#include "DirectionalLight.h"

#include "Debug/Debug.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void DirectionalLight::on_enabled()
    {
        Component::on_enabled();
    }

    void DirectionalLight::on_disabled()
    {
        Component::on_disabled();
    }

    void DirectionalLight::on_update([[maybe_unused]] f32 delta) {}

    void DirectionalLight::on_debug_draw(DebugDrawContext& ctx)
    {
        (void)ctx;

        auto start = m_owner->transform.position;
        auto end = start + m_owner->transform.forward();
        Debug::draw_line(start, end, 0.0f, Color::Green);
        Debug::draw_cube(end, m_owner->transform.euler_angles, Vector3::One * 0.1f);
    }

    void DirectionalLight::on_draw(RenderContext& context)
    {
        if (!context.render_flags.test(RenderState::LightCollection))
            return;

        context.directional_lights.push_back(GPUDirectionalLight{
            {
                .direction = Vector4{ -m_owner->transform.forward() },
                .color = light_color,
            },
            split_lambda,
        });
    }

    void DirectionalLight::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(light_color);
        FSN_SERIALIZE_MEMBER(split_lambda);
    }

    void DirectionalLight::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(light_color);
        FSN_DESERIALIZE_MEMBER(split_lambda);
    }
}
