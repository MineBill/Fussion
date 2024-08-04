#pragma once
#include "VulkanDevice.h"
#include "Fussion/RHI/Buffer.h"

namespace Fussion::RHI {
    VkBufferUsageFlags BufferUsageToVulkan(BufferUsageFlags usage);

    class VulkanBuffer final : public Buffer {
    public:
        VulkanBuffer(VulkanDevice* device, BufferSpecification spec);

        virtual void SetData(void* data, size_t size) override;
        virtual void SetData(void const* data, size_t size) override;
        virtual void SetData(void const* data, size_t size, size_t offset) override;

        virtual void* GetMappedData() override;

        virtual void CopyToImage(Ref<Image> const& image) override;

        virtual BufferSpecification const& GetSpec() const override { return m_Specification; }

        virtual void* GetRawHandle() override { return m_Handle; }

    private:
        BufferSpecification m_Specification{};
        VmaAllocation m_Allocation;
        VmaAllocationInfo m_AllocationInfo;
        VkBuffer m_Handle{};
    };
}
