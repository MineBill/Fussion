#pragma once
#include "Fussion/Scene/Component.h"

namespace Fussion {
    class Camera final : public Component {
    public:
        COMPONENT(Camera)

#if FSN_DEBUG_DRAW
        virtual void OnDebugDraw(DebugDrawContext& ctx) override;
#endif
        virtual void OnUpdate(f32 delta) override;

        [[nodiscard]]
        auto GetPerspective() const -> Mat4 { return m_Perspective; }

        f32 Near{ 0.1f }, Far{ 1000.0f };

        f32 Fov{ 50.0f };

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

    private:
        Mat4 m_Perspective{ 1.0f };
    };
}
