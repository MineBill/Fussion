#pragma once
#include "Fussion/Assets/Asset.h"

namespace Fussion {
    class ShaderAsset final : public Asset {
    public:
        // ShaderAsset(
        //     Ref<RHI::RenderPass> const& render_pass,
        //     std::span<RHI::ShaderStage> stages,
        //     RHI::ShaderMetadata const& metadata);
        //
        // static Ref<ShaderAsset> Create(
        //     Ref<RHI::RenderPass> const& render_pass,
        //     std::span<RHI::ShaderStage> stages,
        //     RHI::ShaderMetadata const& metadata);

        static AssetType static_type() { return AssetType::Shader; }
        virtual AssetType type() const override { return static_type(); }

        // Ref<RHI::Shader>& GetShader() { return m_Shader; }
        // Ref<RHI::RenderPass>& AssociatedRenderPass() { return m_TheRenderPass; }

    private:
        // Ref<RHI::Shader> m_Shader;
        // Ref<RHI::RenderPass> m_TheRenderPass{};
        // RHI::ShaderMetadata m_Metadata{};
    };
}
