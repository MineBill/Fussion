#pragma once
#include "Image.h"
#include "RenderPass.h"
#include "Fussion/Math/Vector2.h"

namespace Fussion::RHI {
constexpr s32 MAX_FRAME_BUFFER_ATTACHMENTS = 5;

struct FrameBufferAttachmentInfo {
    ImageFormat Format;
    ImageUsageFlags Usage;
    s32 Samples = 1;
};

struct FrameBufferSpecification {
    s32 Width, Height;
    s32 Samples = 1;
    std::vector<FrameBufferAttachmentInfo> Attachments;
    bool DontCreateImages;
};

class FrameBuffer : public RenderHandle {
public:
    virtual void Destroy() = 0;

    virtual void Resize(Vector2 new_size) = 0;

    virtual Ref<Image> GetColorAttachment(u32 index) = 0;
    virtual Ref<ImageView> GetColorAttachmentAsView(u32 index) = 0;
    virtual Ref<ImageView> GetDepthAttachmentAsView() = 0;

    virtual FrameBufferSpecification GetSpec() = 0;
};
}
