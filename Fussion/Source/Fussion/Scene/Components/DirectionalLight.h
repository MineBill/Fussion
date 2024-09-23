#pragma once
#include <Fussion/Scene/Component.h>

namespace Fussion {
    class [[API]] DirectionalLight final : public Component {
    public:
        COMPONENT_DEFAULT(DirectionalLight)
        COMPONENT_DEFAULT_COPY(DirectionalLight)

        virtual void on_enabled() override;
        virtual void on_disabled() override;
        virtual void on_update(f32 delta) override;

#if FSN_DEBUG_DRAW
        virtual void on_debug_draw(DebugDrawContext& ctx) override;
#endif

        virtual void on_draw(RenderContext& context) override;

        Color light_color{ Color::White };
        f32 split_lambda{ 0.95f };

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };
}
