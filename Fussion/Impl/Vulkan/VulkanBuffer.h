#pragma once
#include "VulkanDevice.h"
#include "Fussion/Renderer/Buffer.h"

namespace Fussion
{
    VkBufferUsageFlags BufferUsageToVulkan(BufferUsageFlags usage);

    class VulkanBuffer: public Buffer
    {
    public:
        VulkanBuffer(VulkanDevice* device, BufferSpecification spec);

        void SetData(void* data, size_t size) override;
        void CopyToImage(Ref<Image> const& image) override;

        BufferSpecification const& GetSpec() const override { return m_Specification; }

        void* GetRawHandle() override { return m_Handle; }

    private:
        BufferSpecification m_Specification{};
        VmaAllocation m_Allocation;
        VmaAllocationInfo m_AllocationInfo;
        VkBuffer m_Handle{};
    };
}