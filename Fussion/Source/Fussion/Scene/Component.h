#pragma once
#include "Fussion/Core/Types.h"

namespace Fussion
{
    class Entity;

    class Component
    {
    public:
        Component() = default;
        explicit Component(Entity* owner): m_Owner(owner) {}
        virtual ~Component() = default;

        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(f32 delta) {}

        Entity* GetOwner() const { return m_Owner; }
    protected:
        Entity* m_Owner;
    };
}

#define COMPONENT(name) \
    explicit name(Entity* owner): Component(owner) {}