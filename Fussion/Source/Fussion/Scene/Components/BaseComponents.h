#pragma once
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Assets/Model.h"
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Log/Log.h"
#include "Fussion/Scene/Component.h"

namespace Fussion {
    class [[API]] PointLight final : public Component {
    public:
        COMPONENT_DEFAULT(PointLight)
        COMPONENT_DEFAULT_COPY(PointLight)

        virtual void on_update(f32 delta) override;
        virtual void on_draw(RenderContext& context) override;

        [[API, BackgroundColor(Color::Red)]]
        f32 radius { 10.0f };
        [[API]]
        Vector3 offset {};

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
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
        virtual void on_debug_draw(DebugDrawContext& ctx) override;
#endif

        Type draw_type { Type::Box };

        f32 size { 0.0f };

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };

    class [[API]] BallSpawner final : public Component {
    public:
        COMPONENT_DEFAULT(BallSpawner)
        COMPONENT_DEFAULT_COPY(BallSpawner)

        virtual void on_update(f32 delta) override;

        [[API]]
        f32 speed { 0.1f };

        [[API]]
        AssetRef<Model> model;

        [[API, EditorButton("Spawn")]]
        void spawn();
        [[API, EditorButton("Clear")]]
        void clear();

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
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

        virtual void on_draw(RenderContext& context) override;

        [[API, EditorRegion("SSAO"), EditorName("SSAO")]]
        bool ssao {};

        [[API, EditorRegion("SSAO"), EditorName("SSAO Radius"), Range(0, 1, 0.1)]]
        f32 ssao_radius {};

        [[API, EditorRegion("SSAO"), EditorName("SSAO Bias"), Range(0, 0.05, 0.001)]]
        f32 ssao_bias {};

        [[API, EditorRegion("SSAO"), EditorName("SSAO Noise Scale"), Range(1, 16, 1)]]
        f32 ssao_noise_scale {};

        [[API, EditorRegion("ToneMapping"), EditorName("Gamma")]]
        f32 gamma {};

        [[API, EditorRegion("ToneMapping"), EditorName("Exposure")]]
        f32 exposure {};

        [[API, EditorRegion("ToneMapping"), EditorName("Tonemap Mode")]]
        TonemapMode tonemap_mode {};

        [[API, EditorName("Env Map")]]
        AssetRef<Texture2D> environment_map;

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };
}

namespace Fsn = Fussion;
