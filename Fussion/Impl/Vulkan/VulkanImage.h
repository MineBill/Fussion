#pragma once
#include "Fussion/RHI/Image.h"
#include "VulkanDevice.h"

#include "volk.h"
#include "VulkanSampler.h"

namespace Fussion::RHI {
class VulkanImageView;
VkFormat ImageFormatToVulkan(ImageFormat format);
VkSampleCountFlagBits SampleCountToVulkan(s32 count);
VkImageUsageFlags ImageUsageToVulkan(ImageUsageFlags flags);
VkImageLayout ImageLayoutToVulkan(ImageLayout layout);

class VulkanImage : public Image {
public:
    VulkanImage(VulkanDevice* device, ImageSpecification spec);
    VulkanImage(VulkanDevice* device, VkImage existing_image, ImageSpecification spec);

    void Destroy() override;

    ImageSpecification const& GetSpec() const override { return Specification; }
    void SetData(std::span<u8>) override;
    void TransitionLayout(ImageLayout new_layout) override;

    void* GetRawHandle() override { return Handle; }

    Ref<VulkanImageView> View{};
    Ref<VulkanSampler> Sampler{};
    ImageSpecification Specification;

    VmaAllocation Allocation;
    VkImage Handle{};

private:
    bool m_DestroyHandle{ false };
};
}
