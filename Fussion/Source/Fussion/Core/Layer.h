#pragma once
#include "Types.h"
#include "Fussion/Events/Event.h"

namespace Fussion
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

namespace Fsn = Fussion;