#pragma once
#include "Types.h"
#include "Engin5/Events/Event.h"

namespace Engin5
{
    class Layer
    {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        virtual void OnStart() {}
        virtual void OnEnable() {}
        virtual void OnDisable() {}

        virtual void OnUpdate(f32) {}
        virtual void OnEvent(Event&) {}
    };
}