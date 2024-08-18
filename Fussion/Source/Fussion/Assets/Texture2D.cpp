#include "FussionPCH.h"
#include "Texture2D.h"

#include "AssetManager.h"
#include "RHI/Device.h"

namespace Fussion {
    Ref<Texture2D> Texture2D::Create(std::span<u8> data, Texture2DMetadata const& metadata)
    {
        Ref<Texture2D> texture = MakeRef<Texture2D>();

        RHI::ImageSpecification image_spec{};
        image_spec.Width = metadata.Width;
        image_spec.Height = metadata.Height;
        // Since normal maps have non-color data, the image format HAS to be UNORM.
        image_spec.Format = metadata.IsNormalMap ? RHI::ImageFormat::R8G8B8A8_UNORM : metadata.Format;
        image_spec.SamplerSpec.Filter = metadata.Filter;
        image_spec.SamplerSpec.UseAnisotropy = false;
        image_spec.SamplerSpec.Wrap = metadata.Wrap;
        image_spec.GenerateMipMaps = true;

        using enum RHI::ImageUsage;
        image_spec.Usage = ColorAttachment | Sampled | TransferDst | TransferSrc;
        image_spec.FinalLayout = RHI::ImageLayout::ShaderReadOnlyOptimal;

        texture->m_Metadata = metadata;
        texture->m_Image = RHI::Device::Instance()->CreateImage(image_spec);
        texture->m_Image->SetData(data);
        texture->m_Image->TransitionLayout(RHI::ImageLayout::ShaderReadOnlyOptimal);

        return texture;
    }
}
