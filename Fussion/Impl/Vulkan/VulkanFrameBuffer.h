#pragma once
#include "VulkanDevice.h"
#include "VulkanImageView.h"
#include "VulkanRenderPass.h"
#include "Fussion/RHI/FrameBuffer.h"

namespace Fussion::RHI {
class VulkanImage;

class VulkanFrameBuffer : public FrameBuffer {
public:
    VulkanFrameBuffer(VulkanDevice* device, Ref<RenderPass> render_pass, FrameBufferSpecification spec);
    VulkanFrameBuffer(
        VulkanDevice* device,
        Ref<RenderPass> render_pass,
        std::vector<Ref<Image>> images,
        FrameBufferSpecification spec);
    VulkanFrameBuffer(
        VulkanDevice* device,
        Ref<RenderPass> render_pass,
        std::vector<Ref<ImageView>> images,
        FrameBufferSpecification spec);

    void Destroy() override;
    void* GetRawHandle() override { return m_Handle; }

    void Resize(Vector2 new_size) override;

    Ref<Image> GetColorAttachment(u32 index) override;
    Ref<ImageView> GetColorAttachmentAsView(u32 index) override;
    Ref<ImageView> GetDepthAttachmentAsView() override;

    FrameBufferSpecification GetSpec() override;

private:
    void Invalidate();

    std::vector<Ref<VulkanImage>> m_ColorAttachments;
    Ref<VulkanImage> m_DepthImage;

    std::vector<Ref<VulkanImageView>> m_ColorAttachmentViews;
    Ref<VulkanImageView> m_DepthImageView;

    std::vector<FrameBufferAttachmentInfo> m_ColorFormats;
    std::optional<FrameBufferAttachmentInfo> m_DepthFormat;

    Ref<VulkanRenderPass> m_RenderPass;
    FrameBufferSpecification m_Specification;
    VkFramebuffer m_Handle;
};
}
