#pragma once
#include "RenderHandle.h"
#include "Fussion/Core/Types.h"

namespace Fussion
{
    enum class ImageFormat
    {
        None,
        R8G8B8A8_SRGB,
        B8G8R8A8_SRGB,
        R8G8B8A8_UNORM,
        B8G8R8A8_UNORM,
        RED_SIGNED,
        RED_UNSIGNED,
        R16G16B16A16_SFLOAT,
        D16_UNORM,
        D32_SFLOAT,
        DEPTH24_STENCIL8,
        DEPTH32_SFLOAT,
        D32_SFLOAT_S8_UINT,
    };

    enum class ImageLayout
    {
        Undefined,
        ColorAttachmentOptimal,
        DepthStencilAttachmentOptimal,
        DepthStencilReadOnlyOptimal,
        ShaderReadOnlyOptimal,
        TransferSrcOptimal,
        TransferDstOptimal,
        PresentSrc,
        AttachmentOptimal,
    };

    enum class RenderPassAttachmentLoadOp
    {
        DontCare,
        Load,
        Clear,
    };

    enum class RenderPassAttachmentStoreOp
    {
        DontCare,
        Store,
    };

    struct RenderPassAttachmentRef
    {
        u32 Attachment;
        ImageLayout Layout;
    };

    struct RenderPassSubPass
    {
        std::vector<RenderPassAttachmentRef> ColorAttachments;
        std::vector<RenderPassAttachmentRef> ResolveAttachments;
        std::vector<RenderPassAttachmentRef> InputAttachments;
        std::optional<RenderPassAttachmentRef> DepthStencilAttachment;
    };

    struct RenderPassAttachment
    {
        std::string Label;

        RenderPassAttachmentLoadOp LoadOp;
        RenderPassAttachmentStoreOp StoreOp;
        ImageFormat Format;
        s32 Samples = 1;

        RenderPassAttachmentLoadOp StencilLoadOp;
        RenderPassAttachmentStoreOp StencilStoreOp;
        s32 StencilClearValue = 0;

        ImageFormat StencilFormat;
        ImageLayout InitialLayout;
        ImageLayout FinalLayout;

        std::array<f32, 4> ClearColor = {1.f, 1.f, 1.f, 1.f};
        f32 ClearDepth = 1.f;
        u32 ClearStencil = 1;
    };

    struct RenderPassSpecification
    {
        std::string Label;
        std::vector<RenderPassAttachment> Attachments;
        std::vector<RenderPassSubPass> SubPasses;
    };

    class RenderPass: public RenderHandle
    {
    public:
        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual RenderPassSpecification GetSpec() = 0;
    };
}