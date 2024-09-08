#include "FussionPCH.h"
#include "VulkanImage.h"
#include <Fussion/Math/Math.h>

#include <magic_enum/magic_enum.hpp>

#include "Common.h"
#include "VulkanCommandBuffer.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"

namespace Fussion::RHI {
    VulkanImage::VulkanImage(VulkanDevice* device, ImageSpecification spec)
        : m_DestroyHandle(true)
    {
        Specification = spec;

        if (spec.GenerateMipMaps) {
            MipLevels = CAST(u32, Math::floor_log2(Math::max(spec.Width, spec.Height))) + 1;
        }
        m_MipLevelsLayouts.resize(MipLevels);

        auto ci = VkImageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = ImageFormatToVulkan(spec.Format),
            .extent = VkExtent3D{
                .width = CAST(u32, spec.Width),
                .height = CAST(u32, spec.Height),
                .depth = 1,
            },
            .mipLevels = MipLevels,
            .arrayLayers = CAST(u32, spec.LayerCount),
            .samples = SampleCountToVulkan(spec.Samples),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = ImageUsageToVulkan(spec.Usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = ImageLayoutToVulkan(spec.Layout),
        };

        auto alloc_info = VmaAllocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        VK_CHECK(vmaCreateImage(device->Allocator, &ci, &alloc_info, &Handle, &Allocation, nullptr))

        auto view_spec = ImageViewSpecification{
            .ViewType = ci.arrayLayers > 1 ? ImageViewType::D2Array : ImageViewType::D2,
            .Format = spec.Format,
            .BaseLayerIndex = 0,
            .LayerCount = spec.LayerCount,
            .MipLevels = MipLevels,
        };

        View = std::make_unique<VulkanImageView>(device, this, view_spec);
        Sampler = std::make_unique<VulkanSampler>(device, spec.SamplerSpec);
    }

    VulkanImage::VulkanImage(VulkanDevice* device, VkImage existing_image, ImageSpecification spec)
        : Specification(spec), Handle(existing_image)
    {
        auto view_spec = ImageViewSpecification{
            .ViewType = ImageViewType::D2,
            .Format = spec.Format,
            .MipLevels = MipLevels,
        };
        m_MipLevelsLayouts.resize(MipLevels);
        View = std::make_unique<VulkanImageView>(device, this, view_spec);
    }

    void VulkanImage::Destroy()
    {
        if (Handle == VK_NULL_HANDLE)
            return;
        if (m_DestroyHandle) {
            auto device = Device::Instance()->As<VulkanDevice>();
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
            .Size = CAST(s32, data.size_bytes()),
            .Mapped = true,
        });

        buffer->SetData(data.data(), data.size());

        // TODO: Remove this operation from the buffer object and add it to the command buffer.
        TransitionLayout(ImageLayout::TransferDstOptimal);
        // TODO: Remove this operation from the buffer object and add it to the command buffer.
        buffer->CopyToImage(this->As<Image>());

        if (Specification.GenerateMipMaps) {
            GenerateMipmaps();
        } else {
            // We only do this in the else because GenerateMipmaps takes care of
            // transitioning each mip level to ShaderReadOnlyOptimal.
            TransitionLayout(ImageLayout::ShaderReadOnlyOptimal);
        }
    }

    void VulkanImage::TransitionLayout(ImageLayout new_layout)
    {
        auto old_layout = Specification.Layout;
        if (old_layout == new_layout) {
            return;
        }

        Specification.Layout = new_layout;
        auto cmd = Device::Instance()->BeginSingleTimeCommand()->As<VulkanCommandBuffer>();
        defer(Device::Instance()->EndSingleTimeCommand(cmd));

        auto barrier = VkImageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout = ImageLayoutToVulkan(old_layout),
            .newLayout = ImageLayoutToVulkan(new_layout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = Handle,
            .subresourceRange = VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = MipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        for (u32 i = 0; i < MipLevels; i++) {
            m_MipLevelsLayouts[i] = barrier.newLayout;
        }

        auto src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        auto dst_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        using enum ImageLayout;
        if (old_layout == Undefined && new_layout == TransferDstOptimal) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            src_stage = VK_PIPELINE_STAGE_HOST_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == Undefined && new_layout == TransferSrcOptimal) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_HOST_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == TransferSrcOptimal && new_layout == TransferDstOptimal) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == TransferDstOptimal && new_layout == TransferSrcOptimal) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == TransferDstOptimal && new_layout == ShaderReadOnlyOptimal) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (old_layout == ShaderReadOnlyOptimal && new_layout == TransferDstOptimal) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == ShaderReadOnlyOptimal && new_layout == TransferSrcOptimal) {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == TransferSrcOptimal && new_layout == ShaderReadOnlyOptimal) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (old_layout == ColorAttachmentOptimal && new_layout == ShaderReadOnlyOptimal) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (old_layout == ColorAttachmentOptimal && new_layout == TransferSrcOptimal) {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == TransferSrcOptimal && new_layout == ColorAttachmentOptimal) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } else {
            PANIC("Unsupported image transition from '{}' to '{}'", magic_enum::enum_name(old_layout), magic_enum::enum_name(new_layout));
        }

        vkCmdPipelineBarrier(cmd->Handle, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void VulkanImage::GenerateMipmaps()
    {
        if (MipLevels == 1) return;
        auto cmd = Device::Instance()->BeginSingleTimeCommand();
        defer(Device::Instance()->EndSingleTimeCommand(cmd));

        auto barrier = VkImageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            // .oldLayout = ImageLayoutToVulkan(old_layout),
            // .newLayout = ImageLayoutToVulkan(new_layout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = Handle,
            .subresourceRange = VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1, // Leave this to one because we only deal with 1 level at a time.
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        s32 width = Specification.Width;
        s32 height = Specification.Height;

        for (u32 i = 1; i < MipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            m_MipLevelsLayouts[i - 1] = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            Specification.Layout = ImageLayout::TransferSrcOptimal;
            vkCmdPipelineBarrier(cmd->GetRenderHandle<VkCommandBuffer>(),
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            auto image = this->As<Image>();
            cmd->BlitImage(
                image,
                image,
                Rect::from_size(CAST(f32, width), CAST(f32, height)),
                Rect::from_size(Math::max(CAST(f32, width / 2), 1.0f), Math::max(CAST(f32, height / 2), 1.0f)),
                i - 1, i);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            m_MipLevelsLayouts[i - 1] = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            Specification.Layout = ImageLayout::ShaderReadOnlyOptimal;
            vkCmdPipelineBarrier(cmd->GetRenderHandle<VkCommandBuffer>(),
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (width > 1) width /= 2;
            if (height > 1) height /= 2;
        }

        barrier.subresourceRange.baseMipLevel = MipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        m_MipLevelsLayouts[MipLevels - 1] = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        Specification.Layout = ImageLayout::ShaderReadOnlyOptimal;
        vkCmdPipelineBarrier(cmd->GetRenderHandle<VkCommandBuffer>(),
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    VkImageLayout VulkanImage::GetLayoutForMipLevel(u32 level) const
    {
        VERIFY(level < m_MipLevelsLayouts.size());
        return m_MipLevelsLayouts[level];
    }

    VkFormat ImageFormatToVulkan(ImageFormat format)
    {
        using enum ImageFormat;
        switch (format) {
        case R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case B8G8R8A8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case RED_SIGNED:
            return VK_FORMAT_R8_SINT;
        case RED_UNSIGNED:
            return VK_FORMAT_R8_UINT;
        case R16G16B16A16_SFLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case D32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case DEPTH32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case D32_SFLOAT_S8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case None:
        case DEPTH24_STENCIL8:
        default:
            break;
        }
        return {};
    }

    VkSampleCountFlagBits SampleCountToVulkan(s32 count)
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

    VkImageUsageFlags ImageUsageToVulkan(ImageUsageFlags flags)
    {
        VkImageUsageFlags ret{ 0 };
        if (flags.test(ImageUsage::ColorAttachment))
            ret |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (flags.test(ImageUsage::DepthStencilAttachment))
            ret |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (flags.test(ImageUsage::Sampled))
            ret |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (flags.test(ImageUsage::Storage))
            ret |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (flags.test(ImageUsage::TransferSrc))
            ret |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (flags.test(ImageUsage::TransferDst))
            ret |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (flags.test(ImageUsage::Input))
            ret |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        if (flags.test(ImageUsage::Transient))
            ret |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        return ret;
    }

    VkImageLayout ImageLayoutToVulkan(ImageLayout layout)
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

        UNREACHABLE;
    }
}
