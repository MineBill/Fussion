#include "e5pch.h"
#include "Texture2D.h"

#include "RHI/Device.h"
#include "Vulkan/Common.h"

namespace Fussion {
    Ref<Texture2D> Texture2D::Create(std::span<u8> data, Texture2DSpec spec)
    {
        Ref<Texture2D> texture = MakeRef<Texture2D>();
        RHI::ImageSpecification image_spec{};
        image_spec.Width = spec.Width;
        image_spec.Height = spec.Height;
        image_spec.Format = RHI::ImageFormat::R8G8B8A8_UNORM;
        image_spec.SamplerSpec.Filter = RHI::FilterMode::Linear;
        image_spec.SamplerSpec.UseAnisotropy = false;
        image_spec.SamplerSpec.Wrap = RHI::WrapMode::Repeat;

        using enum RHI::ImageUsage;
        image_spec.Usage = ColorAttachment | Sampled | TransferDst | TransferSrc;
        image_spec.FinalLayout = RHI::ImageLayout::ShaderReadOnlyOptimal;

        texture->m_Spec = spec;
        texture->m_Image = RHI::Device::Instance()->CreateImage(image_spec);
        texture->m_Image->SetData(data);
        texture->m_Image->TransitionLayout(RHI::ImageLayout::ShaderReadOnlyOptimal);

        return texture;
    }
}
