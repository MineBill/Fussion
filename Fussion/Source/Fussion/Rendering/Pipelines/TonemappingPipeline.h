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
        void init(Vector2 size, GPU::TextureFormat output_format);

        void process(GPU::CommandEncoder& encoder, GPU::TextureView& output, RenderContext const& render_context);
        void resize(Vector2 size);

        auto view() -> GPU::TextureView&;

    private:
        GPU::ShaderModule m_shader {};
        GPU::BindGroupLayout m_bind_group_layout {};
        GPU::BindGroup m_bind_group {};

        GPU::BindGroupLayout m_settings_bgl {};
        GPU::BindGroup m_settings_bg {};

        GPU::RenderPipeline m_pipeline {};
        GPU::Texture m_render_texture {};
        GPU::Sampler m_sampler {};

        UniformBuffer<PostProcessing::Tonemapping> m_tonemapping_buffer {};
    };
}
