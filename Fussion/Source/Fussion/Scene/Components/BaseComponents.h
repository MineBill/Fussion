#pragma once
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Assets/Model.h>
#include <Fussion/Assets/Texture2D.h>
#include <Fussion/Core/Types.h>
#include <Fussion/Scene/Component.h>

namespace Fussion {
    class [[API]] PointLight final : public Component {
    public:
        COMPONENT_DEFAULT(PointLight)
        COMPONENT_DEFAULT_COPY(PointLight)

        virtual void OnUpdate(f32 delta) override;
        virtual void OnDraw(RenderContext& context) override;

        [[API, BackgroundColor(Color::Red)]]
        f32 radius { 10.0f };
        [[API]]
        Vector3 offset {};

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };

    class [[API]] DebugDrawer final : public Component {
    public:
        enum class Type {
            Sphere,
            Box,
        };

        COMPONENT_DEFAULT(DebugDrawer)
        COMPONENT_DEFAULT_COPY(DebugDrawer)

#if FSN_DEBUG_DRAW
        virtual void OnDebugDraw(DebugDrawContext& ctx) override;
#endif

        Type draw_type { Type::Box };

        f32 size { 0.0f };

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };

    class [[API]] BallSpawner final : public Component {
    public:
        COMPONENT_DEFAULT(BallSpawner)
        COMPONENT_DEFAULT_COPY(BallSpawner)

        virtual void OnUpdate(f32 delta) override;

        [[API]]
        f32 speed { 0.1f };

        [[API]]
        AssetRef<Model> model;

        [[API, EditorButton("Spawn")]]
        void spawn();
        [[API, EditorButton("Clear")]]
        void clear();

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };

    class [[API]] Environment final : public Component {
    public:
        COMPONENT_DEFAULT(Environment)
        COMPONENT_DEFAULT_COPY(Environment)

        enum class [[API]] TonemapMode : u32 {
            None = 0,
            ACES = 1,
            Reinhard = 2,
        };

        virtual void OnDraw(RenderContext& context) override;

        [[API, EditorRegion("SSAO"), EditorName("SSAO")]]
        bool UseSSAO {};

        [[API, EditorRegion("SSAO"), EditorName("SSAO Radius"), Range(0, 1, 0.1)]]
        f32 SSAORadius { 0.5f };

        [[API, EditorRegion("SSAO"), EditorName("SSAO Bias"), Range(0, 0.05, 0.001)]]
        f32 SSAOBias { 0.025f };

        [[API, EditorRegion("SSAO"), EditorName("SSAO Noise Scale"), Range(1, 16, 1)]]
        f32 SSAONoiseScale { 2.0f };

        [[API, EditorRegion("ToneMapping"), EditorName("Gamma")]]
        f32 TMGamma { 2.2f };

        [[API, EditorRegion("ToneMapping"), EditorName("Exposure")]]
        f32 TMExposure { 1.0f };

        [[API, EditorRegion("ToneMapping"), EditorName("Tonemap Mode")]]
        TonemapMode TMMode { TonemapMode::Reinhard };

        [[API, EditorName("Env Map")]]
        AssetRef<Texture2D> EnvironmentMap;

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };
}

namespace Fsn = Fussion;
