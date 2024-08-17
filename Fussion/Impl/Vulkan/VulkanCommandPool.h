#pragma once
#include "VulkanDevice.h"
#include "volk.h"
#include "Fussion/RHI/CommandPool.h"

namespace Fussion::RHI {
    class VulkanCommandPool : public CommandPool {
    public:
        VulkanCommandPool(VulkanDevice* device);

        virtual auto AllocateCommandBuffer(CommandBufferSpecification const& spec) -> Ref<CommandBuffer> override;
        virtual auto AllocateCommandBuffers(u32 count, CommandBufferSpecification const& spec) -> std::vector<Ref<CommandBuffer>> override;

        virtual auto GetRawHandle() -> void* override;

        VkCommandPool Handle{};
    };
}
