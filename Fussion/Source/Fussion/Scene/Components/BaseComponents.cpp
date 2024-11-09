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
            .position = m_owner->transform.Position,
            .light_color = Color::White,
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
            Debug::draw_cube(m_owner->transform.Position, m_owner->transform.EulerAngles, Vector3::One * size);
        } else if (draw_type == Type::Sphere) {
            Debug::draw_sphere(m_owner->transform.Position, m_owner->transform.EulerAngles, size);
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
        m_owner->transform.Position.x += delta * speed;
    }

    void BallSpawner::spawn()
    {
        for (u32 x = 0; x < 10; ++x) {
            for (u32 y = 0; y < 10; ++y) {
                auto new_entity = m_owner->scene().create_entity("Test", m_owner->handle());
                auto mr = new_entity->add_component<MeshRenderer>();
                mr->ModelAsset = model;
                new_entity->transform.Position = Vector3(x, Math::sin((x + y) / 50.0f), y);

                auto mat = make_ref<PbrMaterial>();
                mat->object_color = Color::Red;
                mat->roughness = CAST(f32, x) / 10.0f;
                mat->metallic = CAST(f32, y) / 10.0f;
                auto mat_ref = AssetManager::create_virtual_asset_ref<PbrMaterial>(mat);

                mr->Materials.push_back(mat_ref);
            }
        }
    }

    void BallSpawner::clear()
    {
        auto children = m_owner->children();
        for (auto child : children) {
            m_owner->scene().destroy_entity(child);
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

        context.post_processing_settings.use_ssao = UseSSAO;
        context.post_processing_settings.ssao_data.radius = SSAORadius;
        context.post_processing_settings.ssao_data.bias = SSAOBias;
        context.post_processing_settings.ssao_data.noise_scale = SSAONoiseScale;

        context.post_processing_settings.tonemapping_settings.gamma = TMGamma;
        context.post_processing_settings.tonemapping_settings.exposure = TMExposure;
        context.post_processing_settings.tonemapping_settings.mode = CAST(u32, TMMode);

        context.environment_texture = EnvironmentMap.get();
    }

    void Environment::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(UseSSAO);
        FSN_SERIALIZE_MEMBER(SSAOBias);
        FSN_SERIALIZE_MEMBER(SSAONoiseScale);
        FSN_SERIALIZE_MEMBER(SSAORadius);
        FSN_SERIALIZE_MEMBER(TMGamma);
        FSN_SERIALIZE_MEMBER(TMExposure);
        FSN_SERIALIZE_MEMBER(TMMode);

        FSN_SERIALIZE_MEMBER(EnvironmentMap);
    }

    void Environment::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(UseSSAO);
        FSN_DESERIALIZE_MEMBER(SSAOBias);
        FSN_DESERIALIZE_MEMBER(SSAONoiseScale);
        FSN_DESERIALIZE_MEMBER(SSAORadius);
        FSN_DESERIALIZE_MEMBER(TMGamma);
        FSN_DESERIALIZE_MEMBER(TMExposure);
        FSN_DESERIALIZE_MEMBER(TMMode);

        FSN_DESERIALIZE_MEMBER(EnvironmentMap);
    }
}
