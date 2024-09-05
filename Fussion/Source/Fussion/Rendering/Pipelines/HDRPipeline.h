#pragma once
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class HDRPipeline {
    public:
        constexpr static auto Format = GPU::TextureFormat::RGBA16Float;

        /// Initialize the HDR pipeline.
        /// @param output_format The format of the view this pipeline will render to.
        void Init(Vector2 size, GPU::TextureFormat output_format);

        void Process(GPU::CommandEncoder& encoder, GPU::TextureView& output);

        void Resize(Vector2 size);
        auto View() -> GPU::TextureView&;

    private:
        GPU::ShaderModule m_Shader{};
        GPU::BindGroupLayout m_BindGroupLayout{};
        GPU::BindGroup m_BindGroup{};

        GPU::RenderPipeline m_Pipeline{};
        GPU::Texture m_RenderTexture{};
        GPU::Sampler m_Sampler{};
    };
}
