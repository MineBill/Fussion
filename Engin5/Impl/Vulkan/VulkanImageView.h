#pragma once
#include "Engin5/Renderer/Image.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"

#include "volk.h"

namespace Engin5
{
    VkImageViewType ImageViewTypeToVulkan(ImageViewType type);

    class VulkanImageView: public ImageView
    {
    public:
        VulkanImageView(const VulkanDevice* device, const VulkanImage* image, ImageViewSpecification);
        void Destroy() override;

        ImageViewSpecification const& GetSpec() override { return m_Specification; }
        void* GetRawHandle() override { return m_Handle; }

    private:
        ImageViewSpecification m_Specification{};
        VkImageView m_Handle{};
    };
}