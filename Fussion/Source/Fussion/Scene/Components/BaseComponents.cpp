#include "BaseComponents.h"

#include "Assets/AssetManager.h"
#include "Debug/Debug.h"
#include "MeshRenderer.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void PointLight::OnUpdate(f32) { }

    void PointLight::OnDraw(RenderContext& context)
    {
        if (!context.RenderFlags.test(RenderState::LightCollection))
            return;
        auto light = GPUPointLight {
            .Position = m_Owner->Transform.Position,
            .LightColor = Color::White,
            .Radius = radius,
        };
        context.PointLights.push_back(light);
    }

    void PointLight::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(offset);
        FSN_SERIALIZE_MEMBER(radius);
    }

    void PointLight::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(offset);
        FSN_DESERIALIZE_MEMBER(radius);
    }

    void DebugDrawer::OnDebugDraw(DebugDrawContext& ctx)
    {
        (void)ctx;

        if (draw_type == Type::Box) {
            Debug::DrawCube(m_Owner->Transform.Position, m_Owner->Transform.EulerAngles, Vector3::One * size);
        } else if (draw_type == Type::Sphere) {
            Debug::DrawSphere(m_Owner->Transform.Position, m_Owner->Transform.EulerAngles, size);
        }
    }

    void DebugDrawer::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(size);
        FSN_SERIALIZE_MEMBER(draw_type);
    }

    void DebugDrawer::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(size);
        FSN_DESERIALIZE_MEMBER(draw_type);
    }

    void BallSpawner::OnUpdate(f32 delta)
    {
        m_Owner->Transform.Position.x += delta * speed;
    }

    void BallSpawner::spawn()
    {
        for (u32 x = 0; x < 10; ++x) {
            for (u32 y = 0; y < 10; ++y) {
                auto new_entity = m_Owner->GetScene().CreateEntity("Test", m_Owner->GetHandle());
                auto mr = new_entity->AddComponent<MeshRenderer>();
                mr->ModelAsset = model;
                new_entity->Transform.Position = Vector3(x, Math::Sin((x + y) / 50.0f), y);

                auto mat = MakeRef<PbrMaterial>();
                mat->object_color = Color::Red;
                mat->roughness = CAST(f32, x) / 10.0f;
                mat->metallic = CAST(f32, y) / 10.0f;
                auto mat_ref = AssetManager::CreateVirtualAssetRef<PbrMaterial>(mat);

                mr->Materials.push_back(mat_ref);
            }
        }
    }

    void BallSpawner::clear()
    {
        auto children = m_Owner->GetChildren();
        for (auto child : children) {
            m_Owner->GetScene().DestroyEntity(child);
        }
    }

    void BallSpawner::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(speed);
    }

    void BallSpawner::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(speed);
    }

    void Environment::OnDraw(RenderContext& context)
    {
        if (!context.RenderFlags.test(RenderState::LightCollection))
            return;

        context.PostProcessingSettings.UseSSAO = ssao;
        context.PostProcessingSettings.SSAOData.Radius = ssao_radius;
        context.PostProcessingSettings.SSAOData.Bias = ssao_bias;
        context.PostProcessingSettings.SSAOData.NoiseScale = ssao_noise_scale;

        context.PostProcessingSettings.TonemappingSettings.Gamma = gamma;
        context.PostProcessingSettings.TonemappingSettings.Exposure = exposure;
        context.PostProcessingSettings.TonemappingSettings.Mode = CAST(u32, tonemap_mode);

        context.EnvironmentMap = environment_map.Get();
    }

    void Environment::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(ssao);
        FSN_SERIALIZE_MEMBER(ssao_bias);
        FSN_SERIALIZE_MEMBER(ssao_noise_scale);
        FSN_SERIALIZE_MEMBER(ssao_radius);
        FSN_SERIALIZE_MEMBER(gamma);
        FSN_SERIALIZE_MEMBER(exposure);
        FSN_SERIALIZE_MEMBER(tonemap_mode);

        FSN_SERIALIZE_MEMBER(environment_map);
    }

    void Environment::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
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
