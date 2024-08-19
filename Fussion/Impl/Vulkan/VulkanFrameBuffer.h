#pragma once
#include "VulkanDevice.h"
#include "VulkanImageView.h"
#include "VulkanRenderPass.h"
#include "Fussion/RHI/FrameBuffer.h"

namespace Fussion::RHI {
class VulkanImage;

class VulkanFrameBuffer : public FrameBuffer {
public:
    VulkanFrameBuffer(VulkanDevice* device, Ref<RenderPass> const& render_pass, FrameBufferSpecification const& spec);
    VulkanFrameBuffer(
        VulkanDevice* device,
        Ref<RenderPass> const& render_pass,
        std::vector<Ref<Image>> const& images,
        FrameBufferSpecification const& spec);
    VulkanFrameBuffer(
        VulkanDevice* device,
        Ref<RenderPass> const& render_pass,
        std::vector<Ref<ImageView>> const& images,
        FrameBufferSpecification const& spec);

    virtual void Destroy() override;
    virtual auto GetRawHandle() -> void* override { return m_Handle; }

    virtual void Resize(Vector2 new_size) override;

    virtual auto GetColorAttachment(u32 index) -> Ref<Image> override;
    virtual auto GetColorAttachmentAsView(u32 index) -> Ref<ImageView> override;
    virtual auto GetDepthAttachmentAsView() -> Ref<ImageView> override;

    virtual auto GetSpec() -> FrameBufferSpecification const& override;
    virtual auto ReadPixel(Vector2 position, u32 attachment) -> Result<std::array<u8, 4>, Error> override;

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
