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

        context.PostProcessingSettings.UseSSAO = UseSSAO;
        context.PostProcessingSettings.SSAOData.Radius = SSAORadius;
        context.PostProcessingSettings.SSAOData.Bias = SSAOBias;
        context.PostProcessingSettings.SSAOData.NoiseScale = SSAONoiseScale;

        context.PostProcessingSettings.TonemappingSettings.Gamma = TMGamma;
        context.PostProcessingSettings.TonemappingSettings.Exposure = TMExposure;
        context.PostProcessingSettings.TonemappingSettings.Mode = CAST(u32, TMMode);

        context.EnvironmentMap = EnvironmentMap.Get();
    }

    void Environment::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(UseSSAO);
        FSN_SERIALIZE_MEMBER(SSAOBias);
        FSN_SERIALIZE_MEMBER(SSAONoiseScale);
        FSN_SERIALIZE_MEMBER(SSAORadius);
        FSN_SERIALIZE_MEMBER(TMGamma);
        FSN_SERIALIZE_MEMBER(TMExposure);
        FSN_SERIALIZE_MEMBER(TMMode);

        FSN_SERIALIZE_MEMBER(EnvironmentMap);
    }

    void Environment::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
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
