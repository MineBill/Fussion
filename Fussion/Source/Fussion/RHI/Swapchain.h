#pragma once
#include "RenderPass.h"

namespace Fussion::RHI {
    enum class VideoPresentMode {
        Fifo,
        Immediate,
        Mailbox,
    };

    struct SwapChainSpecification {
        Vector2 Size;
        VideoPresentMode PresentMode;
        ImageFormat Format;
    };

    class Swapchain {
    public:
        virtual ~Swapchain() = default;

        virtual auto GetNextImage() -> Maybe<u32> = 0;
        virtual void Present(u32 image) = 0;
        virtual void SubmitCommandBuffer(Ref<CommandBuffer> cmd) = 0;
        virtual void Resize(u32 width, u32 height) = 0;
        virtual auto GetImageCount() const -> u32 = 0;

        virtual auto GetFrameBuffer(u32 index) -> Ref<FrameBuffer> = 0;

        virtual auto GetRenderPass() -> RenderPass* = 0;

        [[nodiscard]]
        virtual auto GetSpec() const -> SwapChainSpecification = 0;
    };
}
