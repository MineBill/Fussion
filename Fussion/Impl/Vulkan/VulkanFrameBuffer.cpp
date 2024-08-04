#include "e5pch.h"
#include "VulkanFrameBuffer.h"
#include "VulkanImageView.h"
#include "VulkanImage.h"
#include "Common.h"
#include "Math/Rect.h"

namespace Fussion::RHI {
    VulkanFrameBuffer::VulkanFrameBuffer([[maybe_unused]] VulkanDevice* device, Ref<RenderPass> render_pass, FrameBufferSpecification spec)
        : m_RenderPass(render_pass->As<VulkanRenderPass>()), m_Specification(spec)
    {
        for (const auto attachment : spec.Attachments) {
            if (Image::IsDepthFormat(attachment.Format)) {
                m_DepthFormat = attachment;
            } else {
                m_ColorFormats.push_back(attachment);
            }
        }
        Invalidate();
    }

    VulkanFrameBuffer::VulkanFrameBuffer(
        [[maybe_unused]] VulkanDevice* device,
        Ref<RenderPass> render_pass,
        std::vector<Ref<Image>> images,
        FrameBufferSpecification spec)
        : m_RenderPass(render_pass->As<VulkanRenderPass>()), m_Specification(spec)
    {
        m_Specification.DontCreateImages = true;
        for (const auto image : images) {
            if (Image::IsDepthFormat(image->GetSpec().Format)) {
                m_DepthImage = image->As<VulkanImage>();
            } else {
                m_ColorAttachments.push_back(image->As<VulkanImage>());
            }
        }

        Invalidate();
    }

    VulkanFrameBuffer::VulkanFrameBuffer(
        [[maybe_unused]] VulkanDevice* device,
        Ref<RenderPass> render_pass,
        std::vector<Ref<ImageView>> images,
        FrameBufferSpecification spec)
        : m_RenderPass(render_pass->As<VulkanRenderPass>()), m_Specification(spec)
    {
        m_Specification.DontCreateImages = true;
        for (const auto image : images) {
            if (Image::IsDepthFormat(image->GetSpec().Format)) {
                m_DepthImageView = image->As<VulkanImageView>();
            } else {
                m_ColorAttachmentViews.push_back(image->As<VulkanImageView>());
            }
        }

        Invalidate();
    }

    void VulkanFrameBuffer::Destroy()
    {
        auto device = Device::Instance()->As<VulkanDevice>();

        if (!m_Specification.DontCreateImages) {
            for (auto const& attachment : m_ColorAttachments) {
                device->DestroyImage(attachment);
            }
            m_ColorAttachments.clear();

            device->DestroyImage(m_DepthImage);
            m_DepthImage = nullptr;
        }

        vkDestroyFramebuffer(device->Handle, m_Handle, nullptr);
    }

    FrameBufferSpecification const& VulkanFrameBuffer::GetSpec()
    {
        return m_Specification;
    }

    auto VulkanFrameBuffer::ReadPixel(Vector2 position, u32 attachment) -> Result<std::array<u8, 4>, Error>
    {
        VERIFY(attachment < m_ColorAttachments.size(), "Attachment index is bigger than the amount of attachments in the framebuffer");

        auto& attachment_spec = m_ColorAttachments[attachment]->GetSpec();
        if (!Rect::FromSize(attachment_spec.Width, attachment_spec.Height).Contains(position)) {
            return Error::PositionOutOfBounds;
        }

        auto spec = BufferSpecification{
            .Label = "ReadPixel::Buffer Copy",
            .Usage = BufferUsage::TransferDestination,
            .Size = attachment_spec.Width * attachment_spec.Height * 4,
            .Mapped = true,
        };

        auto buffer = Device::Instance()->CreateBuffer(spec);
        defer(Device::Instance()->DestroyBuffer(buffer));

        auto& image = m_ColorAttachments[attachment];
        auto old_layout = image->GetSpec().FinalLayout;
        image->TransitionLayout(ImageLayout::TransferSrcOptimal);

        auto cmd = Device::Instance()->BeginSingleTimeCommand();
        cmd->CopyImageToBuffer(m_ColorAttachments[attachment], buffer, Vector2(attachment_spec.Width, attachment_spec.Height));
        Device::Instance()->EndSingleTimeCommand(cmd);

        image->TransitionLayout(old_layout);

        auto* data = TRANSMUTE(u8*, buffer->GetMappedData());

        auto byte_pos = CAST(u32, position.Y) * attachment_spec.Width + CAST(u32, position.X);
        return std::array{ data[byte_pos], data[byte_pos + 1], data[byte_pos + 2], data[byte_pos + 3] };
    }

    void VulkanFrameBuffer::Invalidate()
    {
        auto device = Device::Instance()->As<VulkanDevice>();

        if (!m_Specification.DontCreateImages) {
            if (!m_ColorFormats.empty()) {
                for (auto const& format : m_ColorFormats) {
                    auto spec = ImageSpecification{
                        .Label = "FrameBuffer Color Image",
                        .Width = m_Specification.Width,
                        .Height = m_Specification.Height,
                        .Samples = format.Samples,
                        .Format = format.Format,
                        .Usage = format.Usage,
                        .FinalLayout = ImageLayout::ColorAttachmentOptimal,
                    };
                    m_ColorAttachments.push_back(device->CreateImage(spec)->As<VulkanImage>());
                }
            }

            if (m_DepthFormat) {
                auto spec = ImageSpecification{
                    .Label = "FrameBuffer Depth Image",
                    .Width = m_Specification.Width,
                    .Height = m_Specification.Height,
                    .Samples = m_DepthFormat->Samples,
                    .Format = m_DepthFormat->Format,
                    .Usage = m_DepthFormat->Usage,
                };

                m_DepthImage = device->CreateImage(spec)->As<VulkanImage>();
            }
        }

        std::vector<VkImageView> attachments{};

        if (!m_Specification.DontCreateImages) {
            for (auto const& attachment : m_ColorAttachments) {
                attachments.push_back(attachment->View->GetRenderHandle<VkImageView>());
            }

            if (m_DepthImage != nullptr) {
                attachments.push_back(m_DepthImage->View->GetRenderHandle<VkImageView>());
            }
        } else {
            if (m_ColorAttachments.empty()) {
                for (auto const& attachment : m_ColorAttachmentViews) {
                    attachments.push_back(attachment->GetRenderHandle<VkImageView>());
                }

                if (m_DepthImageView != nullptr) {
                    attachments.push_back(m_DepthImageView->GetRenderHandle<VkImageView>());
                }
            } else {
                for (auto const& attachment : m_ColorAttachments) {
                    attachments.push_back(attachment->View->GetRenderHandle<VkImageView>());
                }

                if (m_DepthImage != nullptr) {
                    attachments.push_back(m_DepthImage->View->GetRenderHandle<VkImageView>());
                }
            }

        }

        auto fb_ci = VkFramebufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_RenderPass->GetRenderHandle<VkRenderPass>(),
            .attachmentCount = CAST(u32, attachments.size()),
            .pAttachments = attachments.data(),
            .width = CAST(u32, m_Specification.Width),
            .height = CAST(u32, m_Specification.Height),
            .layers = 1,
        };

        VK_CHECK(vkCreateFramebuffer(device->Handle, &fb_ci, nullptr, &m_Handle))
    }

    void VulkanFrameBuffer::Resize(Vector2 new_size)
    {
        m_Specification.Width = CAST(s32, new_size.X);
        m_Specification.Height = CAST(s32, new_size.Y);

        Destroy();
        Invalidate();
    }

    Ref<Image> VulkanFrameBuffer::GetColorAttachment(u32 index)
    {
        return m_ColorAttachments[index];
    }

    Ref<ImageView> VulkanFrameBuffer::GetColorAttachmentAsView(u32 index)
    {
        return m_ColorAttachmentViews[index];
    }

    Ref<ImageView> VulkanFrameBuffer::GetDepthAttachmentAsView()
    {
        return m_DepthImageView;
    }
}
