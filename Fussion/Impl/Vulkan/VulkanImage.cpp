﻿#include "e5pch.h"
#include "VulkanImage.h"

#include <magic_enum/magic_enum.hpp>

#include "Common.h"
#include "VulkanCommandBuffer.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"


namespace Fussion
{
    VulkanImage::VulkanImage(VulkanDevice* device, ImageSpecification spec)
        : m_DestroyHandle(true)
    {
        Specification = spec;

        auto ci = VkImageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = ImageFormatToVulkan(spec.Format),
            .extent = VkExtent3D {
                .width = cast(u32, spec.Width),
                .height = cast(u32, spec.Height),
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = cast(u32, spec.LayerCount),
            .samples = SampleCountToVulkan(spec.Samples),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = ImageUsageToVulkan(spec.Usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = ImageLayoutToVulkan(spec.Layout),
        };

        auto alloc_info = VmaAllocationCreateInfo {
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        VK_CHECK(vmaCreateImage(device->Allocator, &ci, &alloc_info, &Handle, &Allocation, nullptr))

        auto view_spec = ImageViewSpecification {
            .ViewType = ci.arrayLayers > 1 ? ImageViewType::D2Array : ImageViewType::D2,
            .Format = spec.Format,
            .BaseLayerIndex = 0,
            .LayerCount = spec.LayerCount,
        };

        View = std::make_unique<VulkanImageView>(device, this, view_spec);
        Sampler = std::make_unique<VulkanSampler>(device, spec.SamplerSpec);
    }

    VulkanImage::VulkanImage(VulkanDevice* device, VkImage existing_image, ImageSpecification spec)
        : Specification(spec), Handle(existing_image)
    {
        auto view_spec = ImageViewSpecification {
            .ViewType =  ImageViewType::D2,
            .Format = spec.Format,
        };
        View = std::make_unique<VulkanImageView>(device, this, view_spec);
    }

    void VulkanImage::Destroy()
    {
        if (Handle == VK_NULL_HANDLE) return;
        if (m_DestroyHandle) {
            const auto device = Device::Instance()->As<VulkanDevice>();
            vmaDestroyImage(device->Allocator, Handle, Allocation);
        }

        if (View)
            View->Destroy();
        if (Sampler)
            Sampler->Destroy();
    }

    void VulkanImage::SetData(std::span<u8> data)
    {
        auto buffer = Device::Instance()->CreateBuffer(
        {
            .Label = "Buffer For Image Copy",
            .Usage = BufferUsage::TransferSource,
            .Size = cast(s32, data.size_bytes()),
            .Mapped = true,
        });

        buffer->SetData(data.data(), data.size());

        TransitionLayout(ImageLayout::TransferDstOptimal);
        buffer->CopyToImage(this->As<Image>());
        TransitionLayout(ImageLayout::ShaderReadOnlyOptimal);
    }

    void VulkanImage::TransitionLayout(ImageLayout new_layout)
    {
        const auto old_layout = Specification.Layout;
        if (old_layout == new_layout) {
            return;
        }

        Specification.Layout = new_layout;
        const auto cmd = Device::Instance()->BeginSingleTimeCommand()->As<VulkanCommandBuffer>();
        defer(Device::Instance()->EndSingleTimeCommand(cmd));

        auto barrier = VkImageMemoryBarrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout = ImageLayoutToVulkan(old_layout),
            .newLayout = ImageLayoutToVulkan(new_layout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = Handle,
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        auto src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        auto dst_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        using enum ImageLayout;
        if (old_layout == Undefined && new_layout == TransferDstOptimal) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            src_stage = VK_PIPELINE_STAGE_HOST_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == TransferDstOptimal && new_layout == ShaderReadOnlyOptimal) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (old_layout == ShaderReadOnlyOptimal && new_layout == TransferDstOptimal) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == ColorAttachmentOptimal && new_layout == ShaderReadOnlyOptimal) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (old_layout == ColorAttachmentOptimal && new_layout == TransferSrcOptimal) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == TransferSrcOptimal && new_layout == ColorAttachmentOptimal) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else {
            PANIC("Unsupported image transition from '{}' to '{}'", magic_enum::enum_name(old_layout), magic_enum::enum_name(new_layout));
        }

        vkCmdPipelineBarrier(cmd->Handle, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    VkFormat ImageFormatToVulkan(const ImageFormat format)
    {
        using enum ImageFormat;
        switch (format) {
        case R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        case R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
        case RED_SIGNED: return VK_FORMAT_R8_SINT;
        case RED_UNSIGNED: return VK_FORMAT_R8_UINT;
        case R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case D16_UNORM: return VK_FORMAT_D16_UNORM;
        case D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
        case DEPTH32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
        case D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case None:
        case DEPTH24_STENCIL8:
        default:
            break;
        }
        return {};
    }

    VkSampleCountFlagBits SampleCountToVulkan(const s32 count)
    {
        switch (count) {
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 2:
            return VK_SAMPLE_COUNT_2_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
        case 8:
            return VK_SAMPLE_COUNT_8_BIT;
        case 16:
            return VK_SAMPLE_COUNT_16_BIT;
        case 32:
            return VK_SAMPLE_COUNT_32_BIT;
        case 64:
            return VK_SAMPLE_COUNT_64_BIT;
        }
        return {};
    }

    VkImageUsageFlags ImageUsageToVulkan(const ImageUsageFlags flags)
    {
        VkImageUsageFlags ret{0};
        if (flags.Test(ImageUsage::ColorAttachment))
            ret |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (flags.Test(ImageUsage::DepthStencilAttachment))
            ret |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (flags.Test(ImageUsage::Sampled))
            ret |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (flags.Test(ImageUsage::Storage))
            ret |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (flags.Test(ImageUsage::TransferSrc))
            ret |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (flags.Test(ImageUsage::TransferDst))
            ret |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (flags.Test(ImageUsage::Input))
            ret |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        if (flags.Test(ImageUsage::Transient))
            ret |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        return ret;
    }

    VkImageLayout ImageLayoutToVulkan(const ImageLayout layout)
    {
        using enum ImageLayout;

        switch (layout) {
        case Undefined:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case ColorAttachmentOptimal:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case DepthStencilAttachmentOptimal:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case DepthStencilReadOnlyOptimal:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case ShaderReadOnlyOptimal:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case TransferSrcOptimal:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case TransferDstOptimal:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case PresentSrc:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case AttachmentOptimal:
            return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        }

        PANIC("Invalid image layout '{}'", cast(s32, layout))
        return {};
    }
}