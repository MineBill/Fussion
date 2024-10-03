#pragma once
#include <Fussion/Scene/Component.h>

namespace Fussion {
    class [[API]] DirectionalLight final : public Component {
    public:
        COMPONENT_DEFAULT(DirectionalLight)
        COMPONENT_DEFAULT_COPY(DirectionalLight)

        virtual void OnEnabled() override;
        virtual void OnDisabled() override;
        virtual void OnUpdate(f32 delta) override;

#if FSN_DEBUG_DRAW
        virtual void OnDebugDraw(DebugDrawContext& ctx) override;
#endif

        virtual void OnDraw(RenderContext& context) override;

        [[API, EditorName("Color")]]
        Color LightColor { Color::White };

        [[API, EditorName("Brightness"), Range(0, 20, 0.1)]]
        f32 Brightness { 1.0f };

        [[API, Region("Shadow"), EditorName("Split Lambda"), Range(0.1, 1.0, 0.05)]]
        f32 SplitLambda { 0.95f };

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };
}
