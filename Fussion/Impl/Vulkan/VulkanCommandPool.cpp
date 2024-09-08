#include "FussionPCH.h"
#include "VulkanCommandPool.h"
#include "Common.h"
#include "VulkanCommandBuffer.h"

namespace Fussion::RHI {
    VulkanCommandPool::VulkanCommandPool(VulkanDevice* device)
    {
        auto const pool_create_info = VkCommandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = CAST(u32, device->FamilyIndices.GraphicsFamily.value())
        };
        LOG_INFOF("Creating command pool with family index: {}", device->FamilyIndices.GraphicsFamily.value());

        VK_CHECK(vkCreateCommandPool(device->Handle, &pool_create_info, nullptr, &Handle))
    }

    auto VulkanCommandPool::AllocateCommandBuffer(CommandBufferSpecification const& spec) -> Ref<CommandBuffer>
    {
        return make_ref<VulkanCommandBuffer>(shared_from_this()->As<VulkanCommandPool>(), spec);
    }

    auto VulkanCommandPool::AllocateCommandBuffers(u32 count, CommandBufferSpecification const& spec) -> std::vector<Ref<CommandBuffer>>
    {
        std::vector<Ref<CommandBuffer>> ret;
        for (u32 i = 0; i < count; i++) {
            ret.push_back(AllocateCommandBuffer(spec));
        }
        return ret;
    }

    auto VulkanCommandPool::GetRawHandle() -> void*
    {
        return Handle;
    }
}
