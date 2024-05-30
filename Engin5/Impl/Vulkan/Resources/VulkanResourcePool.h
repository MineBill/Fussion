#pragma once
#include "Engin5/Renderer/Resources/ResourcePool.h"

#include "../volk.h"
#include "../VulkanDevice.h"

namespace Engin5
{
    VkDescriptorType ResourceTypeToVulkan(ResourceType type);

    class VulkanResourcePool: public ResourcePool
    {
    public:
        VulkanResourcePool(VulkanDevice* device, ResourcePoolSpecification spec);
        auto Allocate(Ref<ResourceLayout> layout, const std::string& name) -> Result<Ref<Resource>, AllocationError> override;

        void Reset() override;
        const ResourcePoolSpecification& GetSpec() override { return m_Specification; }
        void* GetRawHandle() override { return m_Handle; }

    private:
        ResourcePoolSpecification m_Specification{};
        VkDescriptorPool m_Handle{};
    };
}
