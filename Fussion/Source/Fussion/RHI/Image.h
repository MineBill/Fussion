#pragma once
#include "RenderPass.h"
#include "Sampler.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Core/BitFlags.h"

namespace Fussion::RHI
{
    enum class ImageUsage
    {
        None = 1 << 0,
        ColorAttachment = 1 << 1,
        DepthStencilAttachment = 1 << 2,
        Sampled = 1 << 3,
        Storage = 1 << 4,
        TransferSrc = 1 << 5,
        TransferDst = 1 << 6,
        Input = 1 << 7,
        Transient = 1 << 8,
    };

    DECLARE_FLAGS(ImageUsage, ImageUsageFlags)
    DECLARE_OPERATORS_FOR_FLAGS(ImageUsageFlags)

    enum class ImageViewType
    {
        D2,
        D2Array,
        D3,
        CubeMap,
    };

    struct ImageViewSpecification
    {
        ImageViewType ViewType;
        ImageFormat Format; // @note Should be able to deduce this from the image.
        s32 BaseLayerIndex;
        s32 LayerCount = 1;
    };

    class ImageView: public RenderHandle
    {
    public:
        virtual void Destroy() = 0;
        virtual ImageViewSpecification const& GetSpec() = 0;
    };

    struct ImageSpecification
    {
        std::string Label = "Image";
        s32 Width, Height;
        s32 Samples = 1;
        ImageFormat Format;
        ImageUsageFlags Usage;

        ImageLayout Layout = ImageLayout::Undefined, FinalLayout;
        SamplerSpecification SamplerSpec;
        s32 LayerCount = 1;
    };

    class Image: public RenderHandle
    {
    public:
        virtual void Destroy() = 0;

        static bool IsDepthFormat(const ImageFormat format)
        {
            using enum ImageFormat;
            switch (format) {
            case D16_UNORM:
            case D32_SFLOAT:
            case D32_SFLOAT_S8_UINT:
            case DEPTH32_SFLOAT:
            case DEPTH24_STENCIL8:
                return true;
            default:
                return false;
            }
        }

        virtual ImageSpecification const& GetSpec() const = 0;

        virtual void SetData(std::span<u8>) = 0;
        virtual void TransitionLayout(ImageLayout new_layout) = 0;

        s32 GetWidth() const
        {
            return GetSpec().Width;
        }

        s32 GetHeight() const
        {
            return GetSpec().Height;
        }
    };
}

namespace Fsn = Fussion;