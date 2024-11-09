#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Events/Event.h"
#include "Fussion/Log/Log.h"
#include <Fussion/GPU/GPU.h>

#include <source_location>

namespace Fussion {
    class Layer {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        virtual void on_start() {}
        virtual void on_enable() {}
        virtual void on_disable() {}

        virtual void on_update([[maybe_unused]] f32 delta) {}
        virtual void on_event([[maybe_unused]] Event& event) {}

        virtual void on_draw([[maybe_unused]] GPU::CommandEncoder& encoder) {}

        virtual void on_log_received(
            [[maybe_unused]] LogLevel level,
            [[maybe_unused]] std::string_view message,
            [[maybe_unused]] std::source_location const& loc) {}
    };
}
