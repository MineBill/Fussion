#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/GPU/GPU.h"
#include "Fussion/GPU/ShaderProcessor.h"

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

        // Ref<RHI::Shader>& GetShader() { return m_Shader; }
        // Ref<RHI::RenderPass>& AssociatedRenderPass() { return m_TheRenderPass; }

        explicit ShaderAsset(GPU::ShaderProcessor::CompiledShader const& compiledShader, std::vector<GPU::TextureFormat> colorTargetFormats);
        virtual ~ShaderAsset() override;

        GPU::RenderPipeline Pipeline() const { return m_Pipeline; }

        /// Returns a bind group layout by set.
        // NOTE: What about creating an enum { Global, Scene, Object }, instead of using an index?
        auto GetBindGroupLayout(u32 index) -> Maybe<GPU::BindGroupLayout>;
        auto GetColorTargetFormats() const -> std::vector<GPU::TextureFormat> { return m_ColorTargetFormats; }
        auto GetMetadata() const -> GPU::ShaderProcessor::ShaderMetadata const& { return m_Metadata; }

        static AssetType StaticType() { return AssetType::Shader; }
        virtual AssetType Type() const override { return StaticType(); }

    private:
        std::unordered_map<u32, GPU::BindGroupLayout> m_BindGroupLayouts {};
        GPU::RenderPipeline m_Pipeline {};
        GPU::ShaderProcessor::ShaderMetadata m_Metadata {};

        std::vector<GPU::TextureFormat> m_ColorTargetFormats {};
    };
}
