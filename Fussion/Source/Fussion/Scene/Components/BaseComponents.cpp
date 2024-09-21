#include "BaseComponents.h"

#include "Debug/Debug.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void PointLight::on_update(f32) {}

    void PointLight::on_draw(RenderContext& context)
    {
        if (!context.render_flags.test(RenderState::LightCollection))
            return;
        auto light = GPUPointLight{
            .position = m_owner->transform.position,
            .color = Color::White,
            .radius = radius,
        };
        context.point_lights.push_back(light);
    }

    void PointLight::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(offset);
        FSN_SERIALIZE_MEMBER(radius);
    }

    void PointLight::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(offset);
        FSN_DESERIALIZE_MEMBER(radius);
    }

    void DebugDrawer::on_debug_draw(DebugDrawContext& ctx)
    {
        (void)ctx;

        if (draw_type == Type::Box) {
            Debug::draw_cube(m_owner->transform.position, m_owner->transform.euler_angles, Vector3::One * size);
        } else if (draw_type == Type::Sphere) {
            Debug::draw_sphere(m_owner->transform.position, m_owner->transform.euler_angles, size);
        }
    }

    void DebugDrawer::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(size);
        FSN_SERIALIZE_MEMBER(draw_type);
    }

    void DebugDrawer::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(size);
        FSN_DESERIALIZE_MEMBER(draw_type);
    }

    void MoverComponent::on_update(f32 delta)
    {
        m_owner->transform.position.x += delta * speed;
    }

    void MoverComponent::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(speed);
    }

    void MoverComponent::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(speed);
    }

    void Environment::on_draw(RenderContext& context)
    {
        if (!context.render_flags.test(RenderState::LightCollection))
            return;

        context.post_processing.use_ssao = ssao;
    }

    void Environment::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(ssao);
    }

    void Environment::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(ssao);
    }
}
