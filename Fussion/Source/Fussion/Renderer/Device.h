﻿#pragma once
#include "Buffer.h"
#include "CommandBuffer.h"
#include "FrameBuffer.h"
#include "Image.h"
#include "Instance.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Sampler.h"
#include "Shader.h"
#include "Swapchain.h"
#include "Fussion/Core/Types.h"
#include "Resources/Resource.h"

namespace Fussion
{
    class Image;
    class Sampler;

    class Device: public RenderHandle
    {
        friend class Renderer;
    public:
        using ImageCreationCallback = std::function<void(Ref<Image> const&, bool create)>;

        static Device* Create(Instance* instance);

        static Ref<Device>& Instance() { return s_Instance; }

        virtual Ref<RenderPass> CreateRenderPass(RenderPassSpecification spec) = 0;
        virtual Ref<CommandBuffer> CreateCommandBuffer(CommandBufferSpecification spec) = 0;
        virtual std::vector<Ref<CommandBuffer>> CreateCommandBuffers(s32 count, CommandBufferSpecification spec) = 0;
        virtual Ref<Sampler> CreateSampler(SamplerSpecification spec) = 0;
        virtual Ref<Image> CreateImage(ImageSpecification spec) = 0;
        virtual Ref<ImageView> CreateImageView(Ref<Image> image, ImageViewSpecification spec) = 0;
        virtual Ref<Swapchain> CreateSwapchain(Ref<RenderPass> render_pass, SwapChainSpecification spec) = 0;
        virtual Ref<FrameBuffer> CreateFrameBuffer(Ref<RenderPass> render_pass, FrameBufferSpecification spec) = 0;
        virtual Ref<FrameBuffer> CreateFrameBufferFromImages(
            Ref<RenderPass> render_pass,
            std::vector<Ref<Image>> images,
            FrameBufferSpecification spec) = 0;
        virtual Ref<ResourceLayout> CreateResourceLayout(std::span<ResourceUsage> resources) = 0;
        virtual Ref<Shader> CreateShader(
            Ref<RenderPass> render_pass,
            std::span<ShaderStage> stages,
            ShaderMetadata metadata) = 0;
        virtual Ref<Buffer> CreateBuffer(BufferSpecification spec) = 0;
        virtual Ref<ResourcePool> CreateResourcePool(ResourcePoolSpecification spec) = 0;

        virtual void WaitIdle() = 0;

        virtual Ref<CommandBuffer> BeginSingleTimeCommand() = 0;
        virtual void EndSingleTimeCommand(Ref<CommandBuffer> cmd) = 0;
        virtual void RegisterImageCallback(ImageCreationCallback const& callback) = 0;

        virtual void DestroyImage(Ref<Image> const& image) = 0;
        virtual void DestroyImageView(Ref<ImageView> const& image_view) = 0;
        virtual void DestroySampler(Ref<Sampler> const& sampler) = 0;
        virtual void DestroyFrameBuffer(Ref<Image> const& frame_buffer) = 0;

        // virtual void CopyBufferToImage(Ptr<Buffer> buffer, Ptr<Image> image) = 0;
    private:
        static Ref<Device> s_Instance;
    };
}