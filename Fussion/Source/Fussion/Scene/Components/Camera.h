#pragma once
#include "Fussion/Scene/Component.h"

namespace Fussion {
    class [[API]] Camera final : public Component {
    public:
        COMPONENT_DEFAULT(Camera)

#if FSN_DEBUG_DRAW
        virtual void on_debug_draw(DebugDrawContext& ctx) override;
#endif
        virtual void on_update(f32 delta) override;

        [[nodiscard]]
        auto get_perspective() const -> Mat4 { return m_perspective; }

        [[API, Range(0.0f, 10.0f, 0.01f)]]
        f32 near{ 0.1f };
        [[API, Range(100.0f, 1000.0f)]]
        f32 far{ 1000.0f };

        [[API]]
        f32 fov{ 50.0f };

        virtual auto clone() -> Ref<Component> override;
        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;

    private:
        Mat4 m_perspective{ 1.0f };
    };
}
