#include "e5pch.h"
#include "VulkanResource.h"
#include "VulkanResourcePool.h"
#include "../Common.h"

namespace Fussion
{
    VkShaderStageFlags ShaderStagesToVulkan(const ShaderTypeFlags stages)
    {
        VkShaderStageFlags flags = 0;
        if (stages.Test(ShaderType::Vertex)) {
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (stages.Test(ShaderType::Fragment)) {
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (stages.Test(ShaderType::Compute)) {
            flags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
        return flags;
    }

    VulkanResourceLayout::VulkanResourceLayout(VulkanDevice* device, std::span<ResourceUsage> resources)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        for (u32 i = 0; i < resources.size(); i++)
        {
            const auto& resource = resources[i];
            VkDescriptorSetLayoutBinding binding {
                .binding = i,
                .descriptorType = ResourceTypeToVulkan(resource.Type),
                .descriptorCount = CAST(u32, resource.Count),
                .stageFlags = ShaderStagesToVulkan(resource.Stages)
            };
            bindings.push_back(binding);
        }

        const auto ci = VkDescriptorSetLayoutCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = CAST(u32, bindings.size()),
            .pBindings = bindings.data()
        };

        VK_CHECK(vkCreateDescriptorSetLayout(device->Handle, &ci, nullptr, &m_Handle));
    }
}