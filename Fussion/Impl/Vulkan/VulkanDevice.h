#pragma once
#include "VulkanInstance.h"
#include "Fussion/Log/Log.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Renderer/Device.h"
#include "Fussion/Core/Types.h"

#include <optional>
#include "volk.h"
#include "VmaUsage.h"

#if OS_WINDOWS
    // Fuck. You. Windows.
    #undef CreateSemaphore
#endif

namespace Fussion
{
    struct QueueFamilyIndices {
        std::optional<u32> GraphicsFamily;
        std::optional<u32> PresentFamily;

        require_results bool IsComplete() const {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }

        require_results std::vector<u32> GetUniqueIndex() const;
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;

        require_results VkPresentModeKHR ChooseOptimalPresentMode() const {
            for (auto mode: PresentModes) {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return mode;
                }
            }
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        require_results VkSurfaceFormatKHR ChooseOptimalFormat() const {
            for (auto format: Formats) {
                if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
                    return format;
                }
            }
            LOG_WARN("Could not find optimal surface format");
            return Formats[0];
        }
    };

    class VulkanDevice: public Device
    {
    public:
        VkDevice Handle{};
        VkDebugUtilsMessengerEXT DebugMessenger{};
        VkPhysicalDevice PhysicalDevice{};
        VkQueue GraphicsQueue{}, PresentQueue{};
        VkCommandPool CommandPool{};
        QueueFamilyIndices FamilyIndices{};
        SwapChainSupportDetails SwapchainSupport{};
        VmaAllocator Allocator{};

        explicit VulkanDevice(::Fussion::Instance* instance);
        ~VulkanDevice() override;

        Ref<RenderPass> CreateRenderPass(RenderPassSpecification spec) override;
        Ref<CommandBuffer> CreateCommandBuffer(CommandBufferSpecification spec) override;
        std::vector<Ref<CommandBuffer>> CreateCommandBuffers(s32 count, CommandBufferSpecification spec) override;

        Ref<Sampler> CreateSampler(SamplerSpecification spec) override;
        Ref<Image> CreateImage(ImageSpecification spec) override;
        Ref<ImageView> CreateImageView(Ref<Image> image, ImageViewSpecification spec) override;
        Ref<Swapchain> CreateSwapchain(Ref<RenderPass> render_pass, SwapChainSpecification spec) override;
        Ref<FrameBuffer> CreateFrameBuffer(Ref<RenderPass> render_pass, FrameBufferSpecification spec) override;
        Ref<FrameBuffer> CreateFrameBufferFromImages(
            Ref<RenderPass> render_pass,
            std::vector<Ref<Image>> images,
            FrameBufferSpecification spec) override;

        Ref<ResourceLayout> CreateResourceLayout(std::span<ResourceUsage> resources) override;
        Ref<Buffer> CreateBuffer(BufferSpecification spec) override;
        Ref<Shader> CreateShader(Ref<RenderPass> render_pass, std::span<ShaderStage> stages, ShaderMetadata metadata) override;
        Ref<ResourcePool> CreateResourcePool(ResourcePoolSpecification spec) override;

        void WaitIdle() override;

        Ref<CommandBuffer> BeginSingleTimeCommand() override;
        void EndSingleTimeCommand(Ref<CommandBuffer> cmd) override;

        void* GetRawHandle() override { return Handle; }

        void RegisterImageCallback(ImageCreationCallback const& callback) override;

        void DestroyImage(Ref<Image> const& image) override;
        void DestroyImageView(Ref<ImageView> const& image_view) override;
        void DestroySampler(Ref<Sampler> const& sampler) override;
        void DestroyFrameBuffer(Ref<Image> const& frame_buffer) override;

        // ======================
        //    Vulkan Specific
        // ======================

        Ref<PipelineLayout> CreatePipelineLayout(std::vector<Ref<ResourceLayout>> layouts, PipelineLayoutSpecification spec);
        Ref<Pipeline> CreatePipeline(
            Ref<RenderPass> const& render_pass,
            Ref<Shader> const& shader,
            Ref<PipelineLayout> const& layout,
            const PipelineSpecification& spec);

        VkFence CreateFence(bool signaled = true) const;
        VkSemaphore CreateSemaphore() const;
        std::vector<VkFence> CreateFences(s32 count, bool signaled = true) const;
        std::vector<VkSemaphore> CreateSemaphores(s32 count) const;

        void DestroyFences(std::span<VkFence> fences);
        void DestroySemaphores(std::span<VkSemaphore> semaphores);

        void SetHandleName(u64 handle, VkObjectType type, const std::string& name);

        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

        VulkanInstance* Instance{};
    private:
        void SetupDebugCallback();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateCommandPool();
        bool IsDeviceSuitable(VkPhysicalDevice device);
        QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device) const;

        std::vector<ImageCreationCallback> m_ImageCallbacks{};
    };
}
