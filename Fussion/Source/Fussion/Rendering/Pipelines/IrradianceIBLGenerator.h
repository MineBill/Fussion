#pragma once
#include "Fussion/Assets/ShaderAsset.h"
#include "Fussion/Rendering/UniformBuffer.h"

#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class IrradianceIBLGenerator {
    public:
        ~IrradianceIBLGenerator();
        void init();

        auto generate(GPU::Texture const& inputTexture) -> GPU::Texture;

    private:
        auto generate_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture;
        auto generate_convoluted_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture;

        struct ViewData {
            Mat4 view {};
        };
        std::array<UniformBuffer<ViewData>, 6> m_per_face_view_data {};

        Ref<ShaderAsset> m_cube_map_generator_shader {};
        GPU::BindGroup m_bind_group {};

        Ref<ShaderAsset> m_cube_map_convolution_shader {};
        GPU::BindGroup m_conv_bind_group {};

        std::array<GPU::Texture, 6> m_render_textures {};
        GPU::Sampler m_sampler {};

        GPU::Buffer m_cube_vertex_buffer {};

        std::array<Mat4, 6> m_capture_views {};
    };
}
