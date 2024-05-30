#include "e5pch.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
#include "Common.h"
#include "VulkanCommandBuffer.h"
#include "Engin5/Renderer/FrameBuffer.h"

#include <tracy/Tracy.hpp>

namespace Engin5
{
    VkPresentModeKHR PresentModeToVulkan(VideoPresentMode mode)
    {
        switch(mode) {
        using enum VideoPresentMode;
        case Fifo:
            return VK_PRESENT_MODE_FIFO_KHR;
        case Immediate:
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case Mailbox:
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }

        return {};
    }

    VulkanSwapchain::VulkanSwapchain(VulkanDevice* device, Ref<RenderPass> render_pass, SwapChainSpecification spec)
        :m_Specification(spec), m_RenderPass(std::dynamic_pointer_cast<VulkanRenderPass>(std::move(render_pass)))
    {
        // ReSharper disable CppUseStructuredBinding
        const auto cap = device->SwapchainSupport.Capabilities;
        // ReSharper restore CppUseStructuredBinding
        m_ImageCount = cap.minImageCount + 1;
        if (cap.maxImageCount > 0 && m_ImageCount > cap.maxImageCount) {
            m_ImageCount = cap.maxImageCount;
        }

        Invalidate();
    }

    std::tuple<u32, bool> VulkanSwapchain::GetNextImage()
    {
        ZoneScoped;
        auto device = dynamic_cast<VulkanDevice*>(Device::Instance().get());
        vkWaitForFences(device->Handle, 1, &m_InFlightFences[m_CurrentFrame], true, std::numeric_limits<u64>::max());
        vkResetFences(device->Handle, 1, &m_InFlightFences[m_CurrentFrame]);

        u32 image;
        auto result = vkAcquireNextImageKHR(device->Handle, m_Handle, std::numeric_limits<u64>::max(), m_ImageAvailableSemaphores[m_CurrentFrame], nullptr, &image);
        switch (result) {
        case VK_ERROR_OUT_OF_DATE_KHR:
        case VK_SUBOPTIMAL_KHR:
            return {image, false};
        default: ;
        }

        return {image, true};
    }

    void VulkanSwapchain::Present(u32 image)
    {
        ZoneScoped;
        auto device = dynamic_cast<VulkanDevice*>(Device::Instance().get());

        VkSemaphore signal_semaphores[] = {
            m_RenderFinishedSemaphores[m_CurrentFrame]
        };

        auto present_info = VkPresentInfoKHR {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signal_semaphores,
            .swapchainCount = 1,
            .pSwapchains = &m_Handle,
            .pImageIndices = &image,
        };

        VK_CHECK(vkQueuePresentKHR(device->PresentQueue, &present_info))
        m_CurrentFrame = (m_CurrentFrame + 1) % MaxFramesInFlight;
    }

    void VulkanSwapchain::SubmitCommandBuffer(Ref<CommandBuffer> cmd)
    {
        ZoneScoped;
        auto device = dynamic_cast<VulkanDevice*>(Device::Instance().get());
        VkSemaphore wait_semaphores[] = {
            m_ImageAvailableSemaphores[m_CurrentFrame],
        };

        VkSemaphore signal_semaphores[] = {
            m_RenderFinishedSemaphores[m_CurrentFrame]
        };

        VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        auto submite_info = VkSubmitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = wait_semaphores,
            .pWaitDstStageMask = &flags,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd->As<VulkanCommandBuffer>()->Handle,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signal_semaphores,
        };

        VK_CHECK(vkQueueSubmit(device->GraphicsQueue, 1, &submite_info, m_InFlightFences[m_CurrentFrame]))
    }

    void VulkanSwapchain::Resize(u32 width, u32 height)
    {
        ZoneScoped;
        Destroy(true);
        m_Specification.Size.x = width;
        m_Specification.Size.y = height;

        Invalidate();
    }

    Ref<FrameBuffer> VulkanSwapchain::GetFrameBuffer(u32 index)
    {
        return m_FrameBuffers[index];
    }

    SwapChainSpecification VulkanSwapchain::GetSpec() const
    {
        return m_Specification;
    }

    RenderPass* VulkanSwapchain::GetRenderPass()
    {
        return m_RenderPass.get();
    }

    void VulkanSwapchain::Destroy(bool keep_handle)
    {
        ZoneScoped;
        auto device =  Device::Instance()->As<VulkanDevice>();

        for (const auto& fb : m_FrameBuffers) {
            fb->Destroy();
        }
        m_FrameBuffers.clear();

        for (const auto& image : m_Images) {
            image->Destroy();
        }
        m_Images.clear();

        m_DepthImage->Destroy();

        device->DestroyFences(m_InFlightFences);
        device->DestroySemaphores(m_ImageAvailableSemaphores);
        device->DestroySemaphores(m_RenderFinishedSemaphores);
    }

    void VulkanSwapchain::Invalidate()
    {
        ZoneScoped;
        auto device = Device::Instance()->As<VulkanDevice>();

        // @note This is here to satisfy the Vulkan Validation Shitlayers.
        (void)device->QuerySwapChainSupport(device->PhysicalDevice);

        auto swapchain_ci = VkSwapchainCreateInfoKHR {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = device->Instance->Surface,
            .minImageCount = m_ImageCount,
            .imageFormat = ImageFormatToVulkan(m_Specification.Format),
            .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = VkExtent2D {
                .width = cast(u32, m_Specification.Size.x),
                .height = cast(u32, m_Specification.Size.y),
            },
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = device->SwapchainSupport.Capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = PresentModeToVulkan(m_Specification.PresentMode),
            .clipped = VK_TRUE,
            .oldSwapchain = m_Handle,
        };

        VK_CHECK(vkCreateSwapchainKHR(device->Handle, &swapchain_ci, nullptr, &m_Handle))

        GetImages();

        auto depth_spec = ImageSpecification {
            .Label = "Swapchain Depth Image",
            .Width = cast(s32, m_Specification.Size.x),
            .Height = cast(s32, m_Specification.Size.y),
            .Samples = 1,
            .Format = ImageFormat::D32_SFLOAT,
            .Usage = ImageUsage::DepthStencilAttachment,
            .FinalLayout = ImageLayout::DepthStencilReadOnlyOptimal,
        };

        m_DepthImage = device->CreateImage(depth_spec)->As<VulkanImage>();

        CreateFrameBuffers();

        m_InFlightFences = device->CreateFences(MaxFramesInFlight);
        m_ImageAvailableSemaphores = device->CreateSemaphores(MaxFramesInFlight);
        m_RenderFinishedSemaphores = device->CreateSemaphores(MaxFramesInFlight);
    }

    void VulkanSwapchain::GetImages()
    {
        ZoneScoped;
        auto device = dynamic_cast<VulkanDevice*>(Device::Instance().get());

        u32 image_count = 0;
        vkGetSwapchainImagesKHR(device->Handle, m_Handle, &image_count, nullptr);

        std::vector<VkImage> images;
        images.resize(image_count);
        m_Images.resize(image_count);

        vkGetSwapchainImagesKHR(device->Handle, m_Handle, &image_count, images.data());

        for (u32 i = 0; i < image_count; i++) {
            auto spec = ImageSpecification {
                .Label = "Swapchain Image",
                .Width = cast(s32, m_Specification.Size.x),
                .Height = cast(s32, m_Specification.Size.y),
                .Samples = 1,
                .Format = m_Specification.Format,
                .Usage = ImageUsage::ColorAttachment | ImageUsage::TransferDst,
                .FinalLayout = ImageLayout::ColorAttachmentOptimal,
            };
            m_Images[i] = std::make_shared<VulkanImage>(device, images[i], spec);
        }
    }

    void VulkanSwapchain::CreateFrameBuffers()
    {
        ZoneScoped;
        const auto device = Device::Instance()->As<VulkanDevice>();

        for (const auto& image : m_Images) {
            const auto spec = FrameBufferSpecification {
                .Width = cast(s32, m_Specification.Size.x),
                .Height = cast(s32, m_Specification.Size.y),
            };

            const std::vector<Ref<Image>> images = {
                image, m_DepthImage,
            };

            m_FrameBuffers.push_back(
                device->CreateFrameBufferFromImages(m_RenderPass, images, spec)->As<VulkanFrameBuffer>());
        }
    }

    u32 VulkanSwapchain::GetImageCount() const {
        return m_Images.size();
    }
}