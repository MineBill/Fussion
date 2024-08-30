#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/Rendering/RenderTypes.h"
#include "Fussion/Serialization/ISerializable.h"

#define FSN_DEBUG_DRAW 1

#if FSN_DEBUG_DRAW
#include <Fussion/Debug/Debug.h>
#endif

namespace Fussion {
    class Entity;
    class Scene;

    class Component : public ISerializable {
        META_HPP_ENABLE_POLY_INFO()
        friend Entity;
        friend Scene;

    public:
        Component() = default;
        explicit Component(Entity* owner): m_Owner(owner) {}
        virtual ~Component() override = default;

        virtual void OnCreate()
        {
            OnStart();
        }

        virtual void OnDestroy() {}

        virtual void OnStart() {}
        virtual void OnUpdate([[maybe_unused]] f32 delta) {}

#if FSN_DEBUG_DRAW
        virtual void OnDebugDraw([[maybe_unused]] DebugDrawContext& ctx) {}
#endif

        virtual void OnEnabled() {}
        virtual void OnDisabled() {}

        virtual void OnDraw([[maybe_unused]] RenderContext& context) {}

        virtual Ref<Component> Clone() { return nullptr; }

        Entity* GetOwner() const { return m_Owner; }

    protected:
        Entity* m_Owner;
    };
}

#define COMPONENT(name)                               \
    META_HPP_ENABLE_POLY_INFO(Component)              \
    public:                                           \
    explicit name(Entity* owner): Component(owner) {}

#define COMPONENT_DEFAULT(name) \
    name() = default;           \
    COMPONENT(name)

#define COMPONENT_DEFAULT_COPY(name) \
    virtual Ref<Component> Clone() override { return MakeRef<name>(); }
