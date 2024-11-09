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

        GPU::RenderPipeline pipeline() const { return m_pipeline; }

        /// Returns a bind group layout by set.
        // NOTE: What about creating an enum { Global, Scene, Object }, instead of using an index?
        auto get_bind_group_layout_for(u32 index) -> Maybe<GPU::BindGroupLayout>;
        auto color_target_formats() const -> std::vector<GPU::TextureFormat> { return m_color_target_formats; }
        auto metadata() const -> GPU::ShaderProcessor::ShaderMetadata const& { return m_metadata; }

        static AssetType static_type() { return AssetType::Shader; }
        virtual AssetType type() const override { return static_type(); }

    private:
        std::unordered_map<u32, GPU::BindGroupLayout> m_bind_group_layouts {};
        GPU::RenderPipeline m_pipeline {};
        GPU::ShaderProcessor::ShaderMetadata m_metadata {};

        std::vector<GPU::TextureFormat> m_color_target_formats {};
    };
}
