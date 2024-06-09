#pragma once
#include "Engin5/Core/Types.h"
#include "Reflect/Reflect.h"
#include "Generated/Component_reflect_generated.h"
REFLECT_CPP_INCLUDE("Engin5/Scene/Entity.h")

namespace Engin5
{
    class Entity;

    REFLECT_CLASS()
    class Component: REFLECT_BASE
    {
        REFLECT_GENERATED_BODY()
    public:
        Component() = default;
        explicit Component(Entity* owner): m_Owner(owner) {}
        virtual ~Component() = default;

        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(f32 delta) {}

        REFLECT_PROPERTY()
        Entity* GetOwner() const { return m_Owner; }
    protected:
        Entity* m_Owner;
    };
}

#define COMPONENT(name) \
    explicit name(Entity* owner): Component(owner) {}