#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/RHI/RenderContext.h"

namespace Fussion
{
    class Entity;

    class Component
    {
        META_HPP_ENABLE_POLY_INFO()
    public:
        Component() = default;
        explicit Component(Entity* owner): m_Owner(owner) {}
        virtual ~Component() = default;

        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate([[maybe_unused]] f32 delta) {}

        virtual void OnEnabled() {}
        virtual void OnDisabled() {}

        virtual void OnDraw([[maybe_unused]] RHI::RenderContext& context) {}

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
