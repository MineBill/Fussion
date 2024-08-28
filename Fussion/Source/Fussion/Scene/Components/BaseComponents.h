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

        virtual void OnUpdate(f32 delta) override;
        virtual void OnDraw(RenderContext& context) override;

        f32 Radius{ 10.0f };
        Vector3 Offset{};

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
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
        virtual void OnDebugDraw(DebugDrawContext& ctx) override;
#endif

        Type DrawType{ Type::Box };

        f32 Size{ 0.0f };

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };

    class MoverComponent final : public Component {
    public:
        COMPONENT_DEFAULT(MoverComponent)
        COMPONENT_DEFAULT_COPY(MoverComponent)

        virtual void OnUpdate(f32 delta) override;

        f32 Speed{ 0.1f };

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };
}

namespace Fsn = Fussion;
