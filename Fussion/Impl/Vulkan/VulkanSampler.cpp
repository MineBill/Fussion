#include "e5pch.h"
#include "VulkanSampler.h"
#include "Common.h"

#include <magic_enum/magic_enum.hpp>

#undef max
#undef min

namespace Fussion::RHI {
VkFilter TextureFilterToVulkan(const FilterMode mode)
{
    using enum FilterMode;
    switch (mode) {
    case Linear:
        return VK_FILTER_LINEAR;
    case Nearest:
        return VK_FILTER_NEAREST;
    }

    VERIFY(false, "Unhandled {} for {}", magic_enum::enum_name(mode), magic_enum::enum_type_name<FilterMode>())
    return {};
}

VkSamplerAddressMode TextureWrapToVulkan(const WrapMode mode)
{
    using enum WrapMode;

    switch (mode) {
    case ClampToEdge:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case ClampToBorder:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case MirrorRepeat:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }

    VERIFY(false, "Unhandled {} for {}", magic_enum::enum_name(mode), magic_enum::enum_type_name<WrapMode>())
    return {};
}

VulkanSampler::VulkanSampler(VulkanDevice* device, SamplerSpecification spec)
    : m_Specification(spec)
{
    auto sampler_create_info = VkSamplerCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = TextureFilterToVulkan(spec.Filter),
        .minFilter = TextureFilterToVulkan(spec.Filter),
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = TextureWrapToVulkan(spec.Wrap),
        .addressModeV = TextureWrapToVulkan(spec.Wrap),
        .addressModeW = TextureWrapToVulkan(spec.Wrap),
        .mipLodBias = 0.0,
        .anisotropyEnable = CAST(VkBool32, spec.UseAnisotropy),
        .maxAnisotropy = spec.UseAnisotropy ? 1.0f : 0.0f,
        .compareEnable = CAST(VkBool32, spec.UseDepthCompare),
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0,
        .maxLod = std::numeric_limits<f32>::max(),
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = false,
    };

    VK_CHECK(vkCreateSampler(device->Handle, &sampler_create_info, nullptr, &m_Handle))
}

void VulkanSampler::Destroy()
{
    auto device = Device::Instance()->As<VulkanDevice>();
    vkDestroySampler(device->Handle, m_Handle, nullptr);
}

}
