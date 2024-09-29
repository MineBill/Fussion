#pragma once
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class Texture2D;
    /// Converts an HDR projected image into a cube texture with 6 faces.
    class CubeSkybox {
    public:
        ~CubeSkybox();
        void init(std::vector<GPU::BindGroupLayout> layouts);
        void execute(GPU::RenderPassEncoder& encoder);

        void set_map(GPU::Texture const& map);

    private:
        GPU::ShaderModule m_shader {};
        GPU::RenderPipeline m_pipeline {};
        GPU::BindGroupLayout m_bind_group_layout {};
        GPU::BindGroup m_bind_group {};

        GPU::Texture m_cube_texture {};
        GPU::Sampler m_sampler {};

        GPU::Buffer m_cube_vertices {};
    };
}
