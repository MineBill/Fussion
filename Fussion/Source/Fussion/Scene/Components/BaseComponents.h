#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Core/Uuid.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Log/Log.h"

namespace Fussion {
    class PointLight final : public Component {
    public:
        COMPONENT_DEFAULT(PointLight)
        COMPONENT_DEFAULT_COPY(PointLight)

        virtual void on_update(f32 delta) override;
        virtual void on_draw(RenderContext& context) override;

        f32 radius{ 10.0f };
        Vector3 offset{};

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };

    class DebugDrawer final : public Component {
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

        Type draw_type{ Type::Box };

        f32 size{ 0.0f };

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };

    class MoverComponent final : public Component {
    public:
        COMPONENT_DEFAULT(MoverComponent)
        COMPONENT_DEFAULT_COPY(MoverComponent)

        virtual void on_update(f32 delta) override;

        f32 speed{ 0.1f };

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };

    class Environment final : public Component {
    public:
        COMPONENT_DEFAULT(Environment)
        COMPONENT_DEFAULT_COPY(Environment)

        virtual void on_draw(RenderContext& context) override;

        bool ssao{};

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };
}

namespace Fsn = Fussion;
