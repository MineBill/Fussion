#include "FussionPCH.h"
#include "ShaderAsset.h"

#include "RHI/Device.h"

namespace Fussion {
    ShaderAsset::ShaderAsset(Ref<RHI::RenderPass> const& render_pass, std::span<RHI::ShaderStage> stages, RHI::ShaderMetadata const& metadata)
        : m_TheRenderPass(render_pass), m_Metadata(metadata)
    {
        m_Shader = RHI::Device::Instance()->CreateShader(render_pass, stages, metadata);
    }

    Ref<ShaderAsset> ShaderAsset::Create(
        Ref<RHI::RenderPass> const& render_pass,
        std::span<RHI::ShaderStage> stages,
        RHI::ShaderMetadata const& metadata)
    {
        auto shader = make_ref<ShaderAsset>(render_pass, stages, metadata);
        return shader;
    }
}
