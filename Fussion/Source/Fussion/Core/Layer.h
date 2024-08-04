#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Events/Event.h"
#include "Fussion/Log/Log.h"
#include "Fussion/RHI/CommandBuffer.h"

#include <source_location>

namespace Fussion {
    class Layer {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        virtual void OnStart() {}
        virtual void OnEnable() {}
        virtual void OnDisable() {}

        virtual void OnUpdate([[maybe_unused]] f32 delta) {}
        virtual void OnEvent([[maybe_unused]] Event& event) {}

        virtual void OnDraw([[maybe_unused]] Ref<RHI::CommandBuffer> const& cmd) {}

        virtual void OnLogReceived(
            [[maybe_unused]] LogLevel level,
            [[maybe_unused]] std::string_view message,
            [[maybe_unused]] std::source_location const& loc) {}
    };
}
