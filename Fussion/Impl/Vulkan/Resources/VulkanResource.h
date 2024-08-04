#pragma once
#include "Fussion/RHI/Resources/Resource.h"

#include "../volk.h"
#include "../VulkanDevice.h"

namespace Fussion::RHI {
    VkShaderStageFlags ShaderStagesToVulkan(ShaderTypeFlags stages);

    class VulkanResourceLayout final : public ResourceLayout {
    public:
        VulkanResourceLayout() = default;
        VulkanResourceLayout(VulkanDevice* device, std::span<ResourceUsage> resources);
        virtual ~VulkanResourceLayout() override;

        virtual void Destroy() override;

        virtual void* GetRawHandle() override { return m_Handle; }

    private:
        VkDescriptorSetLayout m_Handle{};
    };

    class VulkanResource final : public Resource {
        friend class VulkanResourcePool;

    public:
        VulkanResource() = default;
        VulkanResource(VkDescriptorSet const handle) : m_Handle(handle) {}
        virtual ~VulkanResource() override;

        virtual void* GetRawHandle() override { return m_Handle; }

    private:
        VkDescriptorSet m_Handle{};
        Ref<ResourceLayout> m_Layout{};
    };
}
