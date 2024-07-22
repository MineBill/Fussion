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

    // @todo Figure out a better way to return errors.
    virtual std::tuple<u32, bool> GetNextImage() = 0;
    virtual void Present(u32 image) = 0;
    virtual void SubmitCommandBuffer(Ref<CommandBuffer> cmd) = 0;
    virtual void Resize(u32 width, u32 height) = 0;
    virtual u32 GetImageCount() const = 0;

    virtual Ref<FrameBuffer> GetFrameBuffer(u32 index) = 0;

    virtual RenderPass* GetRenderPass() = 0;
    require_results virtual SwapChainSpecification GetSpec() const = 0;
};
}
