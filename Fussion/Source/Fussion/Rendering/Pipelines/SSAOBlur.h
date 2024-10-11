#pragma once
#include "Fussion/Assets/AssetRef.h"
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    class ShaderAsset;
    class SSAOBlur {
    public:
        constexpr static auto Format = GPU::TextureFormat::R16Float;

        void Init(Vector2 const& size);

        void Resize(Vector2 const& new_size, GPU::Texture const& ssao_texture);
        void Render(GPU::CommandEncoder const& encoder, GPU::QuerySet const& set, u32 begin, u32 end);

        auto GetRenderTarget() -> GPU::Texture { return m_RenderTarget; }

    private:
        GPU::Texture m_RenderTarget {};
        GPU::Sampler m_Sampler {};

        GPU::BindGroup m_BindGroup {};
        AssetRef<ShaderAsset> m_Shader {};
    };
}
