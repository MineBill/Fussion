#pragma once
#include "VulkanDevice.h"
#include "VulkanFrameBuffer.h"
#include "VulkanRenderPass.h"
#include "Fussion/RHI/Swapchain.h"

namespace Fussion::RHI {

    class VulkanImage;

    VkPresentModeKHR PresentModeToVulkan(VideoPresentMode mode);

    class VulkanSwapchain final : public Swapchain {
    public:
        VulkanSwapchain(VulkanDevice* device, Ref<RenderPass> render_pass, SwapChainSpecification const& spec);

        virtual auto GetNextImage() -> Maybe<u32> override;
        virtual void Present(u32 image) override;
        virtual void SubmitCommandBuffer(Ref<CommandBuffer> cmd) override;
        virtual void Resize(u32 width, u32 height) override;
        virtual auto GetImageCount() const -> u32 override;

        /// @copydoc Swapchain::GetCurrentFrame()
        virtual u32 GetCurrentFrame() const override { return m_CurrentFrame; }

        virtual auto GetFrameBuffer(u32 index) -> Ref<FrameBuffer> override;

        [[nodiscard]]
        virtual auto GetSpec() const -> SwapChainSpecification override;
        virtual auto GetRenderPass() -> RenderPass* override;

    private:
        void Destroy(bool keep_handle);

        void Invalidate();
        void GetImages();
        void CreateFrameBuffers();

        SwapChainSpecification m_Specification{};
        Ref<VulkanRenderPass> m_RenderPass;
        u32 m_ImageCount{ 0 };
        u32 m_CurrentFrame{ 0 };

        std::vector<Ref<VulkanImage>> m_Images;
        Ref<VulkanImage> m_DepthImage;

        std::vector<Ref<VulkanFrameBuffer>> m_FrameBuffers;

        std::vector<VkFence> m_InFlightFences;
        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;

        VkSwapchainKHR m_Handle{};
    };
}
