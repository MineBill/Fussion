#pragma once
#include "Fussion/Rendering/UniformBuffer.h"

#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class IBLIrradiance {
    public:
        ~IBLIrradiance();
        void init();

        auto generate(GPU::Texture const& input_texture) -> GPU::Texture;

    private:
        auto generate_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& input_texture) -> GPU::Texture;
        auto generate_convoluted_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& input_texture) -> GPU::Texture;

        struct ViewData {
            Mat4 view {};
        };
        std::array<UniformBuffer<ViewData>, 6> m_view_data {};

        GPU::ShaderModule m_shader {};
        GPU::RenderPipeline m_pipeline {};
        GPU::BindGroupLayout m_bind_group_layout {};
        GPU::BindGroup m_bind_group {};

        GPU::ShaderModule m_conv_shader {};
        GPU::RenderPipeline m_conv_pipeline {};
        GPU::BindGroupLayout m_conv_bind_group_layout {};
        GPU::BindGroup m_conv_bind_group {};

        std::array<GPU::Texture, 6> m_render_textures {};
        GPU::Sampler m_sampler {};

        GPU::Buffer m_cube_vertices {};

        std::array<Mat4, 6> m_capture_views {};
    };
}
