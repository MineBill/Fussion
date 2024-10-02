#include "BaseComponents.h"

#include "Assets/AssetManager.h"
#include "Debug/Debug.h"
#include "MeshRenderer.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void PointLight::on_update(f32) { }

    void PointLight::on_draw(RenderContext& context)
    {
        if (!context.render_flags.test(RenderState::LightCollection))
            return;
        auto light = GPUPointLight {
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

    void BallSpawner::on_update(f32 delta)
    {
        m_owner->transform.position.x += delta * speed;
    }

    void BallSpawner::spawn()
    {
        for (u32 x = 0; x < 10; ++x) {
            for (u32 y = 0; y < 10; ++y) {
                auto new_entity = m_owner->scene().create_entity("Test", m_owner->handle());
                auto mr = new_entity->add_component<MeshRenderer>();
                mr->model_asset = model;
                new_entity->transform.position = Vector3(x, Math::sin((x + y) / 50.0f), y);

                auto mat = make_ref<PbrMaterial>();
                mat->object_color = Color::Red;
                mat->roughness = CAST(f32, x) / 10.0f;
                mat->metallic = CAST(f32, y) / 10.0f;
                auto mat_ref = AssetManager::create_virtual_asset_ref<PbrMaterial>(mat);

                mr->materials.push_back(mat_ref);
            }
        }
    }

    void BallSpawner::clear()
    {
        auto children = m_owner->children();
        for (auto child : children) {
            m_owner->scene().destroy(child);
        }
    }

    void BallSpawner::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(speed);
    }

    void BallSpawner::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(speed);
    }

    void Environment::on_draw(RenderContext& context)
    {
        if (!context.render_flags.test(RenderState::LightCollection))
            return;

        context.post_processing.use_ssao = ssao;
        context.post_processing.ssao.radius = ssao_radius;
        context.post_processing.ssao.bias = ssao_bias;
        context.post_processing.ssao.noise_scale = ssao_noise_scale;

        context.post_processing.tonemapping_settings.gamma = gamma;
        context.post_processing.tonemapping_settings.exposure = exposure;
        context.post_processing.tonemapping_settings.mode = CAST(u32, tonemap_mode);

        context.environment_map = environment_map.get();
    }

    void Environment::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(ssao);
        FSN_SERIALIZE_MEMBER(ssao_bias);
        FSN_SERIALIZE_MEMBER(ssao_noise_scale);
        FSN_SERIALIZE_MEMBER(ssao_radius);
        FSN_SERIALIZE_MEMBER(gamma);
        FSN_SERIALIZE_MEMBER(exposure);
        FSN_SERIALIZE_MEMBER(tonemap_mode);

        FSN_SERIALIZE_MEMBER(environment_map);
    }

    void Environment::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(ssao);
        FSN_DESERIALIZE_MEMBER(ssao_bias);
        FSN_DESERIALIZE_MEMBER(ssao_noise_scale);
        FSN_DESERIALIZE_MEMBER(ssao_radius);
        FSN_DESERIALIZE_MEMBER(gamma);
        FSN_DESERIALIZE_MEMBER(exposure);
        FSN_DESERIALIZE_MEMBER(tonemap_mode);

        FSN_DESERIALIZE_MEMBER(environment_map);
    }
}
