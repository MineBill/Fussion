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
        explicit Component(Entity* owner): m_owner(owner) {}
        virtual ~Component() override = default;

        virtual void on_create()
        {
            on_start();
        }

        virtual void on_destroy() {}

        virtual void on_start() {}
        virtual void on_update([[maybe_unused]] f32 delta) {}

#if FSN_DEBUG_DRAW
        virtual void on_debug_draw([[maybe_unused]] DebugDrawContext& ctx) {}
#endif

        virtual void on_enabled() {}
        virtual void on_disabled() {}

        virtual void on_draw([[maybe_unused]] RenderContext& context) {}

        virtual Ref<Component> clone() { return nullptr; }

        Entity* owner() const { return m_owner; }

    protected:
        Entity* m_owner;
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
    virtual Ref<Component> clone() override { return make_ref<name>(); }
