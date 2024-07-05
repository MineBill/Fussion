#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/meta.hpp/meta_all.hpp"
#include "Fussion/Renderer/RenderContext.h"

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
        virtual void OnUpdate(f32 delta) {}

        virtual void OnDraw(RenderContext& context) {}

        Entity* GetOwner() const { return m_Owner; }
    protected:
        Entity* m_Owner;
    };
}

#define COMPONENT(name) \
    explicit name(Entity* owner): Component(owner) {}

#define COMPONENT_DEFAULT(name) \
    name() = default; \
    COMPONENT(name)
