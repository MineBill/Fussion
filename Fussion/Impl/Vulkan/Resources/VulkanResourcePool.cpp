#include "e5pch.h"
#include "VulkanResourcePool.h"
#include "VulkanResource.h"
#include "Fussion/Core/Result.h"
#include "../Common.h"

namespace Fussion::RHI {
VkDescriptorType ResourceTypeToVulkan(ResourceType type)
{
    switch (type) {
        using enum ResourceType;
    case CombinedImageSampler:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case InputAttachment:
        return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    case UniformBuffer:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case StorageBuffer:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case Sampler:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    PANIC("Unknown resource type")
    return {};
}

VulkanResourcePool::VulkanResourcePool(VulkanDevice* device, ResourcePoolSpecification spec)
    : m_Specification(spec)
{
    std::vector<VkDescriptorPoolSize> limits;
    limits.reserve(spec.ResourceLimits.size());

    for (const auto& limit : spec.ResourceLimits) {
        limits.push_back(VkDescriptorPoolSize{
            .type = ResourceTypeToVulkan(limit.Type),
            .descriptorCount = CAST(u32, limit.Limit),
        });
    }

    auto ci = VkDescriptorPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = CAST(u32, spec.MaxSets),
        .poolSizeCount = CAST(u32, limits.size()),
        .pPoolSizes = limits.data(),
    };

    VK_CHECK(vkCreateDescriptorPool(device->Handle, &ci, nullptr, &m_Handle))

}

auto VulkanResourcePool::Allocate(Ref<ResourceLayout> layout, const std::string& name) -> Result<Ref<Resource>, AllocationError>
{
    auto resource = MakeRef<VulkanResource>();
    VERIFY(resource != nullptr);

    auto handle = layout->GetRenderHandle<VkDescriptorSetLayout>();

    auto alloc_info = VkDescriptorSetAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_Handle,
        .descriptorSetCount = 1,
        .pSetLayouts = &handle,
    };

    auto device = Device::Instance()->As<VulkanDevice>();
    auto result = vkAllocateDescriptorSets(device->Handle, &alloc_info, &resource->m_Handle);
    switch (result) {
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return Err(AllocationError::OutOfMemory);
    case VK_ERROR_FRAGMENTED_POOL:
        return Err(AllocationError::FragmentedPool);
    }

    if (!name.empty()) {
        device->SetHandleName(TRANSMUTE(u64, resource->m_Handle), VK_OBJECT_TYPE_DESCRIPTOR_SET, name);
        device->SetHandleName(
            TRANSMUTE(u64, layout->GetRenderHandle<VkDescriptorSetLayout>()),
            VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
            std::format("Layout - {}", name));
    }

    return Ok(resource->As<Resource>());
}

void VulkanResourcePool::Reset()
{
    auto device = Device::Instance()->GetRenderHandle<VkDevice>();
    VK_CHECK(vkResetDescriptorPool(device, m_Handle, {}));
}
}
