#pragma once
#include "Fussion/Assets/ShaderAsset.h"
#include "Fussion/Rendering/UniformBuffer.h"

#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class IrradianceIBLGenerator {
    public:
        ~IrradianceIBLGenerator();
        void Initialize();

        auto Generate(GPU::Texture const& inputTexture) -> GPU::Texture;

    private:
        auto GenerateCubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture;
        auto GenerateConvolutedCubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture;

        struct ViewData {
            Mat4 View {};
        };
        std::array<UniformBuffer<ViewData>, 6> m_PerFaceViewData {};

        Ref<ShaderAsset> m_CubeMapGeneratorShader {};
        GPU::BindGroup m_BindGroup {};

        Ref<ShaderAsset> m_CubeMapConvolutionShader {};
        GPU::BindGroup m_ConvBindGroup {};

        std::array<GPU::Texture, 6> m_RenderTextures {};
        GPU::Sampler m_Sampler {};

        GPU::Buffer m_CubeVertexBuffer {};

        std::array<Mat4, 6> m_CaptureViews {};
    };
}
