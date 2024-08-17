#pragma once
#include "CommandBuffer.h"
#include "RenderHandle.h"

namespace Fussion::RHI {
    class CommandPool : public RenderHandle {
    public:
        virtual auto AllocateCommandBuffer(CommandBufferSpecification const& spec) -> Ref<CommandBuffer> = 0;
        virtual auto AllocateCommandBuffers(u32 u32, CommandBufferSpecification const& cmd_spec) -> std::vector<Ref<CommandBuffer>> = 0;
    };
}
