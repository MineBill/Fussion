#pragma once
#include "Fussion/Scene/Component.h"

namespace Fussion {
    class Camera final : public Component {
    public:
        COMPONENT_DEFAULT(Camera)

#if FSN_DEBUG_DRAW
        virtual void on_debug_draw(DebugDrawContext& ctx) override;
#endif
        virtual void on_update(f32 delta) override;

        [[nodiscard]]
        auto GetPerspective() const -> Mat4 { return m_Perspective; }

        f32 Near{ 0.1f }, Far{ 1000.0f };

        f32 Fov{ 50.0f };

        virtual auto clone() -> Ref<Component> override;
        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;

    private:
        Mat4 m_Perspective{ 1.0f };
    };
}
