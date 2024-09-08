#pragma once
#include "VulkanInstance.h"

#include "Fussion/Core/Types.h"
#include <Fussion/Core/ThreadProtected.h>
#include <Fussion/Core/Maybe.h>
#include "Fussion/Log/Log.h"
#include "Fussion/RHI/Device.h"
#include <Fussion/RHI/Pipeline.h>

#include "VmaUsage.h"
#include "volk.h"

#if OS_WINDOWS
// Fuck. You. Windows.
#undef CreateSemaphore
#endif

namespace Fussion::RHI {
    struct QueueFamilyIndices {
        Maybe<u32> GraphicsFamily{};
        Maybe<u32> PresentFamily{};
        Maybe<u32> TransferFamily{};

        [[nodiscard]]
        bool IsComplete() const
        {
            return GraphicsFamily.has_value() && PresentFamily.has_value() && TransferFamily.has_value();
        }

        [[nodiscard]]
        std::vector<u32> GetUniqueIndex() const;
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;

        [[nodiscard]]
        VkPresentModeKHR ChooseOptimalPresentMode() const
        {
            for (auto mode : PresentModes) {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return mode;
                }
            }
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        [[nodiscard]]
        VkSurfaceFormatKHR ChooseOptimalFormat() const
        {
            for (auto format : Formats) {
                if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
                    return format;
                }
            }
            LOG_WARN("Could not find optimal surface format");
            return Formats[0];
        }
    };

    class VulkanDevice final : public Device {
    public:
        RenderStats Stats{};
        VkDevice Handle{};
        VkDebugUtilsMessengerEXT DebugMessenger{};
        VkPhysicalDevice PhysicalDevice{};
        VkQueue PresentQueue{};
        ThreadProtected<VkQueue> GraphicsQueue{};
        Maybe<VkQueue> TransferQueue{};

        QueueFamilyIndices FamilyIndices{};
        SwapChainSupportDetails SwapchainSupport{};
        VmaAllocator Allocator{};

        Ref<CommandPool> MainCommandPool;

        explicit VulkanDevice(Ref<RHI::Instance> const& instance);
        virtual ~VulkanDevice();

        virtual Ref<CommandPool> GetMainCommandPool() override;

        virtual auto CreateRenderPass(RenderPassSpecification spec) -> Ref<RenderPass> override;
        virtual auto CreateCommandPool() -> Ref<CommandPool> override;

        // virtual auto CreateCommandBuffer(CommandBufferSpecification spec) -> Ref<CommandBuffer> override;
        // virtual auto CreateCommandBuffers(s32 count, CommandBufferSpecification spec) -> std::vector<Ref<CommandBuffer>> override;

        virtual auto CreateSampler(SamplerSpecification spec) -> Ref<Sampler> override;
        virtual auto CreateImage(ImageSpecification spec) -> Ref<Image> override;
        virtual auto CreateImageView(Ref<Image> image, ImageViewSpecification spec) -> Ref<ImageView> override;
        virtual auto CreateSwapchain(Ref<RenderPass> render_pass, SwapChainSpecification spec) -> Ref<Swapchain> override;
        virtual auto CreateFrameBuffer(Ref<RenderPass> render_pass, FrameBufferSpecification spec) -> Ref<FrameBuffer> override;
        virtual auto CreateFrameBufferFromImages(
            Ref<RenderPass> render_pass,
            std::vector<Ref<Image>> images,
            FrameBufferSpecification spec) -> Ref<FrameBuffer> override;
        virtual auto CreateFrameBufferFromImageViews(
            Ref<RenderPass> render_pass,
            std::vector<Ref<ImageView>> images,
            FrameBufferSpecification spec) -> Ref<FrameBuffer> override;

        virtual auto CreateResourceLayout(std::span<ResourceUsage> resources) -> Ref<ResourceLayout> override;
        virtual auto CreateBuffer(BufferSpecification spec) -> Ref<Buffer> override;
        virtual auto CreateShader(Ref<RenderPass> render_pass, std::span<ShaderStage> stages, ShaderMetadata metadata) -> Ref<Shader> override;
        virtual auto CreateResourcePool(ResourcePoolSpecification spec) -> Ref<ResourcePool> override;

        virtual void WaitIdle() override;

        virtual auto BeginSingleTimeCommand() -> Ref<CommandBuffer> override;
        virtual void EndSingleTimeCommand(Ref<CommandBuffer> cmd) override;

        // virtual void* GetRawHandle() override { return Handle; }

        virtual void RegisterImageCallback(ImageCreationCallback const& callback) override;
        virtual void RegisterImageViewCallback(ImageViewCreationCallback const& callback) override;

        virtual void DestroyBuffer(Ref<Buffer> const& buffer) override;
        virtual void DestroyImage(Ref<Image> const& image) override;
        virtual void DestroyImageView(Ref<ImageView> const& image_view) override;
        virtual void DestroySampler(Ref<Sampler> const& sampler) override;
        virtual void DestroyFrameBuffer(Ref<Image> const& frame_buffer) override;

        virtual auto GetRenderStats() -> RenderStats& override { return Stats; }
        // ======================
        //    Vulkan Specific
        // ======================

        auto CreatePipelineLayout(std::vector<Ref<ResourceLayout>> const& layouts, PipelineLayoutSpecification const& spec) -> Ref<PipelineLayout>;
        auto CreatePipeline(
            Ref<RenderPass> const& render_pass,
            Ref<Shader> const& shader,
            Ref<PipelineLayout> const& layout,
            PipelineSpecification const& spec) -> Ref<Pipeline>;

        auto CreateFence(bool signaled = true) const -> VkFence;
        auto CreateSemaphore() const -> VkSemaphore;
        auto CreateFences(s32 count, bool signaled = true) const -> std::vector<VkFence>;
        auto CreateSemaphores(s32 count) const -> std::vector<VkSemaphore>;

        void DestroyFences(std::span<VkFence> fences);
        void DestroySemaphores(std::span<VkSemaphore> semaphores);

        void SetHandleName(u64 handle, VkObjectType type, std::string const& name);

        auto QuerySwapChainSupport(VkPhysicalDevice device) const -> SwapChainSupportDetails;

        Ref<VulkanInstance> Instance{};

    private:
        void SetupDebugCallback();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        auto IsDeviceSuitable(VkPhysicalDevice device) -> bool;
        auto GetQueueFamilies(VkPhysicalDevice device) const -> QueueFamilyIndices;

        std::vector<ImageCreationCallback> m_ImageCallbacks{};
        std::vector<ImageViewCreationCallback> m_ImageViewCallbacks{};
    };
}
