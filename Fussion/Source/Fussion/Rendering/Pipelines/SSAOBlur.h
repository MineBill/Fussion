#pragma once
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class SSAOBlur {
    public:
        constexpr static auto Format = GPU::TextureFormat::R16Float;

        void init(Vector2 const& size);

        void resize(Vector2 const& new_size, GPU::Texture const& ssao_texture);
        void draw(GPU::CommandEncoder const& encoder);

        auto render_target() -> GPU::Texture { return m_render_target; }

    private:
        GPU::Texture m_render_target{};
        GPU::Sampler m_sampler{};

        GPU::RenderPipeline m_pipeline{};
        GPU::BindGroup m_bind_group{};
        GPU::BindGroupLayout m_bind_group_layout{};
    };
}
