#pragma once
#include "Fussion/Window.h"
#include <Fussion/RHI/RenderHandle.h>

namespace Fussion::RHI {
    class Instance : public RenderHandle {
    public:
        static Ref<Instance> Create(Window const& window);
    };
}
