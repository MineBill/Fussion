#include "e5pch.h"
#include "Texture2D.h"

#include "Renderer/Device.h"
#include "Vulkan/Common.h"

namespace Fussion
{
    Ref<Texture2D> Texture2D::Create(u8* data, Texture2DSpec spec)
    {
        Ref<Texture2D> texture = MakeRef<Texture2D>();
        ImageSpecification image_spec{};
        image_spec.Width = spec.Width;
        image_spec.Height = spec.Height;
        image_spec.Format = ImageFormat::R8G8B8A8_SRGB;
        image_spec.SamplerSpec.Filter = FilterMode::Linear;
        image_spec.SamplerSpec.UseAnisotropy = false;
        image_spec.Usage = ImageUsage::ColorAttachment | ImageUsage::Sampled | ImageUsage::TransferDst;
        image_spec.FinalLayout = ImageLayout::ShaderReadOnlyOptimal;

        texture->m_Spec = spec;
        texture->m_Image = Device::Instance()->CreateImage(image_spec);
        texture->m_Image->SetData({data, cast(size_t, spec.Width * spec.Height * 4)});
        texture->m_Image->TransitionLayout(ImageLayout::ShaderReadOnlyOptimal);

        return texture;
    }
}
