#include "e5pch.h"
#include "VulkanFrameBuffer.h"
#include "VulkanImageView.h"
#include "VulkanImage.h"
#include "Common.h"

namespace Fussion
{
    VulkanFrameBuffer::VulkanFrameBuffer(VulkanDevice* device, Ref<RenderPass> render_pass, FrameBufferSpecification spec)
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
        VulkanDevice* device,
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

    void VulkanFrameBuffer::Destroy()
    {
        auto device = Device::Instance()->As<VulkanDevice>();

        if (!m_Specification.DontCreateImages) {
            for (const auto& attachment : m_ColorAttachments) {
                device->DestroyImage(attachment);
            }
            m_ColorAttachments.clear();

            device->DestroyImage(m_DepthImage);
            m_DepthImage = nullptr;
        }

        vkDestroyFramebuffer(device->Handle, m_Handle, nullptr);
    }

    FrameBufferSpecification VulkanFrameBuffer::GetSpec()
    {
        return m_Specification;
    }

    void VulkanFrameBuffer::Invalidate()
    {
        auto device = Device::Instance()->As<VulkanDevice>();

        if (!m_Specification.DontCreateImages) {
            if (!m_ColorFormats.empty()) {
                for (const auto& format : m_ColorFormats) {
                    const auto spec = ImageSpecification {
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
                const auto spec = ImageSpecification {
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

        for (const auto& attachment : m_ColorAttachments) {
            attachments.push_back(attachment->View->GetRenderHandle<VkImageView>());
        }

        if (m_DepthImage != nullptr) {
            attachments.push_back(m_DepthImage->View->GetRenderHandle<VkImageView>());
        }

        auto fb_ci = VkFramebufferCreateInfo {
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

    Ref<Image> VulkanFrameBuffer::GetColorAttachment(u32 index) {
        return m_ColorAttachments[index];
    }
}