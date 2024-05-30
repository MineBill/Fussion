#include "e5pch.h"
#include "VulkanBuffer.h"
#include "Common.h"

#include <cstring>

namespace Engin5
{
    VulkanBuffer::VulkanBuffer(VulkanDevice* device, BufferSpecification spec)
        : m_Specification(spec)
    {
        const auto buffer_ci = VkBufferCreateInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = cast(VkDeviceSize, spec.Size),
            .usage = BufferUsageToVulkan(spec.Usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        auto allocation_info = VmaAllocationCreateInfo {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        if (spec.Mapped) {
            allocation_info.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }

        VK_CHECK(vmaCreateBuffer(device->Allocator, &buffer_ci, &allocation_info, &m_Handle, &m_Allocation, &m_AllocationInfo))

        // device->SetHandleName(reinterpret_cast<u64>(Handle), VK_OBJECT_TYPE_BUFFER, spec.Label);
        device->SetHandleName(transmute(u64, m_Handle), VK_OBJECT_TYPE_BUFFER, spec.Label);
    }

    void VulkanBuffer::SetData(void* data, size_t size)
    {
        if (!m_Specification.Mapped) return;
        memcpy(m_AllocationInfo.pMappedData, data, size);
    }

    VkBufferUsageFlags BufferUsageToVulkan(BufferUsageFlags usage)
    {
        using enum BufferUsage;
        VkBufferUsageFlags ret{};

        if (usage.Test(Index))
            ret |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (usage.Test(Vertex))
            ret |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (usage.Test(Uniform))
            ret |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (usage.Test(TransferSource))
            ret |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (usage.Test(TransferDestination))
            ret |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        return ret;
    }
}