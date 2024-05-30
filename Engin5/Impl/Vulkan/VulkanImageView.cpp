#include "e5pch.h"
#include "VulkanImageView.h"
#include "Common.h"

namespace Engin5
{
    VulkanImageView::VulkanImageView(const VulkanDevice* device, const VulkanImage* image, const ImageViewSpecification spec)
        : m_Specification(spec)
    {
        VkImageAspectFlags aspect_flag = VK_IMAGE_ASPECT_COLOR_BIT;
        if (Image::IsDepthFormat(spec.Format))
            aspect_flag = VK_IMAGE_ASPECT_DEPTH_BIT;

        const auto ci = VkImageViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image->Handle,
            .viewType = ImageViewTypeToVulkan(spec.ViewType),
            .format = ImageFormatToVulkan(spec.Format),
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = aspect_flag,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = cast(u32, spec.BaseLayerIndex),
                .layerCount = cast(u32, spec.LayerCount),
            }
        };

        VK_CHECK(vkCreateImageView(device->Handle, &ci, nullptr, &m_Handle))
    }

    void VulkanImageView::Destroy()
    {
        auto device = Device::Instance()->As<VulkanDevice>();
        vkDestroyImageView(device->Handle, m_Handle, nullptr);
    }

    VkImageViewType ImageViewTypeToVulkan(const ImageViewType type)
    {
        switch (type) {
        case ImageViewType::D2:
            return VK_IMAGE_VIEW_TYPE_2D;
        case ImageViewType::D3:
            return VK_IMAGE_VIEW_TYPE_3D;
        case ImageViewType::CubeMap:
            return VK_IMAGE_VIEW_TYPE_CUBE;
        case ImageViewType::D2Array:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }

        return {};
    }
}