#pragma once
#include <Fussion/Scene/Component.h>

namespace Fussion {
    class DirectionalLight final : public Component {
    public:
        COMPONENT(DirectionalLight)

        virtual void OnEnabled() override;
        virtual void OnDisabled() override;
        virtual void OnUpdate(f32 delta) override;

#if FSN_DEBUG_DRAW
        virtual void OnDebugDraw(DebugDrawContext& ctx) override;
#endif

        virtual void OnDraw(RHI::RenderContext& context) override;

        Color LightColor{ Color::White };
        f32 SplitLambda{ 0.95f };

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };
}
