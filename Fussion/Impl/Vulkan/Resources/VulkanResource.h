#pragma once
#include "Fussion/Renderer/Resources/Resource.h"

#include "../volk.h"
#include "../VulkanDevice.h"

namespace Fussion
{
    VkShaderStageFlags ShaderStagesToVulkan(ShaderTypeFlags stages);

    class VulkanResourceLayout: public ResourceLayout
    {
    public:
        VulkanResourceLayout() = default;
        VulkanResourceLayout(VulkanDevice* device, std::span<ResourceUsage> resources);

        void* GetRawHandle() override { return m_Handle; }
    private:
        VkDescriptorSetLayout m_Handle{};
    };

    class VulkanResource: public Resource
    {
        friend class VulkanResourcePool;
    public:
        VulkanResource() = default;
        VulkanResource(const VkDescriptorSet handle) : m_Handle(handle) {}
        void* GetRawHandle() override { return m_Handle; }

    private:
        VkDescriptorSet m_Handle{};
    };
}
