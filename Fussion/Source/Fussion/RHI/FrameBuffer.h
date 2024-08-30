#pragma once
#include "Image.h"
#include "RenderPass.h"
#include <Fussion/Core/Result.h>
#include <Fussion/Math/Vector2.h>
#include <Fussion/Math/Color.h>

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
        std::string Label{};
    };

    class FrameBuffer : public RenderHandle {
    public:
        enum class Error {
            PositionOutOfBounds,
        };

        virtual void Destroy() = 0;

        virtual void Resize(Vector2 new_size) = 0;

        virtual auto GetColorAttachment(u32 index) -> Ref<Image> = 0;
        virtual auto GetColorAttachmentAsView(u32 index) -> Ref<ImageView> = 0;
        virtual auto GetDepthAttachmentAsView() -> Ref<ImageView> = 0;

        virtual auto GetSpec() -> FrameBufferSpecification const& = 0;
        virtual auto ReadPixel(Vector2 position, u32 attachment = 0) -> Result<std::array<u8, 4>, Error> = 0;
    };
}
