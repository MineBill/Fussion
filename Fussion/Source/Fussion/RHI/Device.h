#pragma once
#include "Buffer.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "FrameBuffer.h"
#include "Image.h"
#include "Instance.h"
#include "RenderPass.h"
#include "Sampler.h"
#include "Shader.h"
#include "Swapchain.h"
#include "Fussion/Core/Types.h"
#include "Resources/Resource.h"

namespace Fussion::RHI {
    class Image;
    class Sampler;

    extern thread_local Ref<CommandPool> ThreadLocalCommandPool;

    struct RenderStats {
        u32 DrawCalls{};

        void Reset()
        {
            DrawCalls = 0;
        }
    };

    class Device {
    public:
        virtual ~Device() = default;

        using ImageCreationCallback = std::function<void(Ref<Image> const&, bool create)>;
        using ImageViewCreationCallback = std::function<void(Ref<ImageView> const&, Ref<Image> const&, bool create)>;

        static Device* Create(Ref<Instance> const& instance);

        static Ptr<Device>& Instance() { return s_Instance; }
        static void SetInstance(Device* ptr);

        virtual auto GetMainCommandPool() -> Ref<CommandPool> = 0;

        virtual auto CreateRenderPass(RenderPassSpecification spec) -> Ref<RenderPass> = 0;
        virtual auto CreateCommandPool() -> Ref<CommandPool> = 0;

        virtual auto CreateSampler(SamplerSpecification spec) -> Ref<Sampler> = 0;
        virtual auto CreateImage(ImageSpecification spec) -> Ref<Image> = 0;
        virtual auto CreateImageView(Ref<Image> image, ImageViewSpecification spec) -> Ref<ImageView> = 0;
        virtual auto CreateSwapchain(Ref<RenderPass> render_pass, SwapChainSpecification spec) -> Ref<Swapchain> = 0;
        virtual auto CreateFrameBuffer(Ref<RenderPass> render_pass, FrameBufferSpecification spec) -> Ref<FrameBuffer> = 0;
        virtual auto CreateFrameBufferFromImages(
            Ref<RenderPass> render_pass,
            std::vector<Ref<Image>> images,
            FrameBufferSpecification spec) -> Ref<FrameBuffer> = 0;
        virtual auto CreateFrameBufferFromImageViews(
            Ref<RenderPass> render_pass,
            std::vector<Ref<ImageView>> images,
            FrameBufferSpecification spec) -> Ref<FrameBuffer> = 0;
        virtual auto CreateResourceLayout(std::span<ResourceUsage> resources) -> Ref<ResourceLayout> = 0;
        virtual auto CreateShader(
            Ref<RenderPass> render_pass,
            std::span<ShaderStage> stages,
            ShaderMetadata metadata) -> Ref<Shader> = 0;
        virtual auto CreateBuffer(BufferSpecification spec) -> Ref<Buffer> = 0;
        virtual auto CreateResourcePool(ResourcePoolSpecification spec) -> Ref<ResourcePool> = 0;

        virtual void WaitIdle() = 0;

        virtual auto BeginSingleTimeCommand() -> Ref<CommandBuffer> = 0;
        virtual void EndSingleTimeCommand(Ref<CommandBuffer> cmd) = 0;
        virtual void RegisterImageCallback(ImageCreationCallback const& callback) = 0;
        virtual void RegisterImageViewCallback(ImageViewCreationCallback const& callback) = 0;

        virtual void DestroyBuffer(Ref<Buffer> const& buffer) = 0;
        virtual void DestroyImage(Ref<Image> const& image) = 0;
        virtual void DestroyImageView(Ref<ImageView> const& image_view) = 0;
        virtual void DestroySampler(Ref<Sampler> const& sampler) = 0;
        virtual void DestroyFrameBuffer(Ref<Image> const& frame_buffer) = 0;

        virtual auto GetRenderStats() -> RenderStats& = 0;

        template<typename T>
        T* As()
        {
            return CAST(T*, this);
        }

        // virtual void CopyBufferToImage(Ptr<Buffer> buffer, Ptr<Image> image) = 0;
    private:
        static Ptr<Device> s_Instance;
    };
}
