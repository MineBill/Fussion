#include "FussionPCH.h"

#include "Rendering/Renderer.h"

#include "ShaderAsset.h"

namespace Fussion {
    // ShaderAsset::ShaderAsset(Ref<RHI::RenderPass> const& render_pass, std::span<RHI::ShaderStage> stages, RHI::ShaderMetadata const& metadata)
    //     : m_TheRenderPass(render_pass), m_Metadata(metadata)
    // {
    //     m_Shader = RHI::Device::Instance()->CreateShader(render_pass, stages, metadata);
    // }
    //
    // Ref<ShaderAsset> ShaderAsset::Create(
    //     Ref<RHI::RenderPass> const& render_pass,
    //     std::span<RHI::ShaderStage> stages,
    //     RHI::ShaderMetadata const& metadata)
    // {
    //     auto shader = make_ref<ShaderAsset>(render_pass, stages, metadata);
    //     return shader;
    // }

    ShaderAsset::ShaderAsset(GPU::ShaderProcessor::CompiledShader const& compiledShader, std::vector<GPU::TextureFormat> colorTargetFormats)
    {
        m_Metadata = compiledShader.Metadata;
        m_ColorTargetFormats = colorTargetFormats;
        usz shaderOutputCount = compiledShader.Metadata.ColorOutputs.size();
        usz targetFormatCount = colorTargetFormats.size();
        if (shaderOutputCount != targetFormatCount) {
            LOG_ERRORF("Mismatch between fragment shader color targets and provided texture formats: {} vs {}", shaderOutputCount, targetFormatCount);
            if (shaderOutputCount > targetFormatCount) {
                PANIC("Cannot continue. Missing {} target formats", shaderOutputCount - targetFormatCount);
            }

            LOG_WARNF("Provided {} more texture formats than needed, ignoring.", targetFormatCount - shaderOutputCount);
        }

        std::vector<GPU::BindGroupLayout> layouts {};
        for (auto const& [setIndex, set] : compiledShader.Metadata.Uniforms) {
            std::vector<GPU::BindGroupLayoutEntry> entries {};
            for (auto const& [bindingIndex, resource] : set) {
                entries.push_back({
                    .Binding = bindingIndex,
                    .Visibility = resource.Stages,
                    .Type = resource.Type,
                    .Count = CAST(u32, resource.Count),
                });
            }
            auto layout = Renderer::Device().CreateBindGroupLayout({
                .Label = "asd",
                .Entries = entries,
            });
            m_BindGroupLayouts[setIndex] = layout;
            layouts.emplace_back(layout);
        }

        auto layout = Renderer::Device().CreatePipelineLayout({ .BindGroupLayouts = layouts });

        GPU::RenderPipelineSpec spec {
            .Label = "Pipeline"sv,
            .Layout = layout,
            .Vertex = { .AttributeLayouts = {} },
            .Primitive = GPU::PrimitiveState::Default(),
            .DepthStencil = None(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = GPU::FragmentStage {},
            .VertexEntryPointOverride = "main"sv,
            .FragmentEntryPointOverride = "main"sv,
        };
        if (compiledShader.Metadata.UseDepth) {
            spec.DepthStencil = GPU::DepthStencilState::Default();
        }

        if (!compiledShader.Metadata.VertexAttributes.empty()) {
            spec.Vertex.AttributeLayouts.push_back(
                GPU::VertexBufferLayout::Create(const_cast<std::vector<GPU::VertexAttribute>&>(compiledShader.Metadata.VertexAttributes)));
        }

        if (auto pragma = std::ranges::find_if(compiledShader.Metadata.ParsedPragmas, [](GPU::ShaderProcessor::ParsedPragma const& pragma) {
                return pragma.Key == "topology";
            });
            pragma != compiledShader.Metadata.ParsedPragmas.end()) {
            if (pragma->Value == "triangles"sv) {
                spec.Primitive.Topology = GPU::PrimitiveTopology::TriangleList;
            } else if (pragma->Value == "triangle_strip"sv) {
                spec.Primitive.Topology = GPU::PrimitiveTopology::TriangleStrip;
            } else if (pragma->Value == "lines"sv) {
                spec.Primitive.Topology = GPU::PrimitiveTopology::LineList;
            } else {
                // Default is triangle list.
                spec.Primitive.Topology = GPU::PrimitiveTopology::TriangleList;
            }
        }

        u32 i = 0;
        for (auto const& colorOutput : compiledShader.Metadata.ColorOutputs) {
            (void)colorOutput;
            spec.Fragment->Targets.push_back({
                .Format = colorTargetFormats[i++],
                .Blend = compiledShader.Metadata.UseBlending ? GPU::BlendState::Default() : Maybe<GPU::BlendState>(None()),
                .WriteMask = GPU::ColorWrite::All,
            });
        }

        GPU::SpirVShaderSpec vsShaderSpec {
            .Label = "Shader"sv,
            .Data = const_cast<std::vector<u32>&>(compiledShader.VertexStage),
        };
        auto vertexShader = Renderer::Device().CreateShaderModuleSpirV(vsShaderSpec);

        GPU::SpirVShaderSpec fsShaderSpec {
            .Label = "Shader"sv,
            .Data = const_cast<std::vector<u32>&>(compiledShader.FragmentStage),
        };
        auto fragmentShader = Renderer::Device().CreateShaderModuleSpirV(fsShaderSpec);

        m_Pipeline = Renderer::Device().CreateRenderPipeline(vertexShader, fragmentShader, spec);
    }

    Maybe<GPU::BindGroupLayout> ShaderAsset::GetBindGroupLayout(u32 index)
    {
        if (m_BindGroupLayouts.contains(index)) {
            return m_BindGroupLayouts[index];
        }
        return None();
    }
}
