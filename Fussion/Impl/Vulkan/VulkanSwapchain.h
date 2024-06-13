#pragma once
#include "VulkanDevice.h"
#include "VulkanFrameBuffer.h"
#include "VulkanRenderPass.h"
#include "Fussion/Renderer/Swapchain.h"

namespace Fussion
{
    constexpr s32 MaxFramesInFlight = 1;

    class VulkanImage;

    VkPresentModeKHR PresentModeToVulkan(VideoPresentMode mode);

    class VulkanSwapchain: public Swapchain
    {
    public:
        VulkanSwapchain(VulkanDevice* device, Ref<RenderPass> render_pass, SwapChainSpecification spec);

        std::tuple<u32, bool> GetNextImage() override;
        void Present(u32 image) override;
        void SubmitCommandBuffer(Ref<CommandBuffer> cmd) override;
        void Resize(u32 width, u32 height) override;
        u32 GetImageCount() const override;

        Ref<FrameBuffer> GetFrameBuffer(u32 index) override;

        require_results SwapChainSpecification GetSpec() const override;
        RenderPass* GetRenderPass() override;
    private:
        void Destroy(bool keep_handle);

        void Invalidate();
        void GetImages();
        void CreateFrameBuffers();

        SwapChainSpecification m_Specification{};
        Ref<VulkanRenderPass> m_RenderPass;
        u32 m_ImageCount{0};
        u32 m_CurrentFrame{0};

        std::vector<Ref<VulkanImage>> m_Images;
        Ref<VulkanImage> m_DepthImage;

        std::vector<Ref<VulkanFrameBuffer>> m_FrameBuffers;

        std::vector<VkFence> m_InFlightFences;
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;

        VkSwapchainKHR m_Handle{};
    };
}
