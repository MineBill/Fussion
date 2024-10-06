#pragma once
#include "Fussion/Rendering/RenderTypes.h"
#include "Fussion/Rendering/UniformBuffer.h"

#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class TonemappingPipeline {
    public:
        constexpr static auto Format = GPU::TextureFormat::RGBA16Float;

        /// Initialize the HDR pipeline.
        /// @param output_format The format of the view this pipeline will render to.
        void Init(Vector2 size, GPU::TextureFormat output_format);

        void Resize(Vector2 size);
        void Render(GPU::CommandEncoder& encoder, GPU::TextureView& output, RenderContext const& render_context);

        auto GetView() -> GPU::TextureView&;

    private:
        GPU::ShaderModule m_Shader {};
        GPU::BindGroupLayout m_BindGroupLayout {};
        GPU::BindGroup m_BindGroup {};

        GPU::BindGroupLayout m_SettingsBgl {};
        GPU::BindGroup m_SettingsBg {};

        GPU::RenderPipeline m_Pipeline {};
        GPU::Texture m_RenderTexture {};
        GPU::Sampler m_Sampler {};

        UniformBuffer<PostProcessing::Tonemapping> m_TonemappingBuffer {};
    };
}
