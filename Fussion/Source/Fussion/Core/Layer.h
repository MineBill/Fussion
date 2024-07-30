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

    virtual void OnUpdate(f32) {}
    virtual void OnEvent(Event&) {}

    virtual void OnDraw([[maybe_unused]] Ref<RHI::CommandBuffer> cmd) {}

    virtual void OnLogReceived(
        [[maybe_unused]] LogLevel level,
        [[maybe_unused]] std::string_view message,
        [[maybe_unused]] std::source_location const& loc) {}
};
}
