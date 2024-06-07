#pragma once
#include "Engin5/Core/Types.h"
#include "Reflect/Reflect.h"
#include "Generated/Component_reflect_generated.h"

namespace Engin5
{
    class Entity;

    REFLECT_CLASS()
    class Component: REFLECT_BASE
    {
        REFLECT_GENERATED_BODY()
    public:
        Component(Entity* owner) {}

        virtual void OnUpdate(f32 delta) {};
    };
}

#define COMPONENT(name) \
    explicit name(Entity* owner): Component(owner) {}