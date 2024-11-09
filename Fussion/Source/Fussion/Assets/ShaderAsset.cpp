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
        m_metadata = compiledShader.metadata;
        m_color_target_formats = colorTargetFormats;
        usz shaderOutputCount = compiledShader.metadata.color_outputs.size();
        usz targetFormatCount = colorTargetFormats.size();
        if (shaderOutputCount != targetFormatCount) {
            LOG_ERRORF("Mismatch between fragment shader color targets and provided texture formats: {} vs {}", shaderOutputCount, targetFormatCount);
            if (shaderOutputCount > targetFormatCount) {
                PANIC("Cannot continue. Missing {} target formats", shaderOutputCount - targetFormatCount);
            }

            LOG_WARNF("Provided {} more texture formats than needed, ignoring.", targetFormatCount - shaderOutputCount);
        }

        std::vector<GPU::BindGroupLayout> layouts {};
        for (auto const& [setIndex, set] : compiledShader.metadata.uniforms) {
            std::vector<GPU::BindGroupLayoutEntry> entries {};
            for (auto const& [bindingIndex, resource] : set) {
                entries.push_back({
                    .binding = bindingIndex,
                    .visibility = resource.stages,
                    .type = resource.type,
                    .count = CAST(u32, resource.count),
                });
            }
            auto layout = Renderer::device().create_bind_group_layout({
                .Label = "asd"sv,
                .entries = entries,
            });
            m_bind_group_layouts[setIndex] = layout;
            layouts.emplace_back(layout);
        }

        auto layout = Renderer::device().create_pipeline_layout({ .bind_group_layouts = layouts });

        GPU::RenderPipelineSpec spec {
            .label = "Pipeline"sv,
            .layout = layout,
            .vertex = { .attribute_layouts = {} },
            .primitive = GPU::PrimitiveState::default_(),
            .depth_stencil = None(),
            .multi_sample = GPU::MultiSampleState::default_(),
            .fragment = None(),
            .vertex_entry_point_override = "main"sv,
            .fragment_entry_point_override = "main"sv,
        };

        if (compiledShader.metadata.depth_state) {
            spec.depth_stencil = compiledShader.metadata.depth_state.unwrap();
        } else if (compiledShader.metadata.use_depth) {
            spec.depth_stencil = GPU::DepthStencilState::default_();
        }

        if (!compiledShader.metadata.vertex_attributes.empty()) {
            spec.vertex.attribute_layouts.push_back(
                GPU::VertexBufferLayout::create(compiledShader.metadata.vertex_attributes)
            );
        }

        if (auto pragma = std::ranges::find_if(compiledShader.metadata.parsed_pragmas, [](GPU::ShaderProcessor::ParsedPragma const& parsedPragma) {
                return parsedPragma.key == "topology";
            });
            pragma != compiledShader.metadata.parsed_pragmas.end()) {
            if (pragma->value == "triangles"sv) {
                spec.primitive.topology = GPU::PrimitiveTopology::TriangleList;
            } else if (pragma->value == "triangle_strip"sv) {
                spec.primitive.topology = GPU::PrimitiveTopology::TriangleStrip;
            } else if (pragma->value == "lines"sv) {
                spec.primitive.topology = GPU::PrimitiveTopology::LineList;
            } else {
                // Default is triangle list.
                spec.primitive.topology = GPU::PrimitiveTopology::TriangleList;
            }
        }

        if (!compiledShader.metadata.color_outputs.empty()) {
            spec.fragment = GPU::FragmentStage {};
        }

        u32 i = 0;
        for (auto const& colorOutput : compiledShader.metadata.color_outputs) {
            (void)colorOutput;
            spec.fragment->targets.push_back({
                .format = colorTargetFormats[i++],
                .blend_state = compiledShader.metadata.use_blending ? GPU::BlendState::default_() : Maybe<GPU::BlendState>(None()),
                .write_mask = GPU::ColorWrite::All,
            });
        }

        GPU::SpirVShaderSpec vsShaderSpec {
            .label = "Shader"sv,
            .data = const_cast<std::vector<u32>&>(compiledShader.vertex_stage),
        };
        auto vertexShader = Renderer::device().create_shader_module_spir_v(vsShaderSpec);

        GPU::SpirVShaderSpec fsShaderSpec {
            .label = "Shader"sv,
            .data = const_cast<std::vector<u32>&>(compiledShader.fragment_stage),
        };
        auto fragmentShader = Renderer::device().create_shader_module_spir_v(fsShaderSpec);

        m_pipeline = Renderer::device().create_render_pipeline(vertexShader, fragmentShader, spec);
    }

    ShaderAsset::~ShaderAsset()
    {
        // NOTE: During shader reload, it's possible that the renderer will use the pipeline
        // as the reload happens, during which the older shader asset destructor is called and the
        // pipeline is destroyed. For now, we avoid destroying the pipeline, because live shader reload
        // is much more valuable than being correct.
        // m_Pipeline.Release();
    }

    Maybe<GPU::BindGroupLayout> ShaderAsset::get_bind_group_layout_for(u32 index)
    {
        if (m_bind_group_layouts.contains(index)) {
            return m_bind_group_layouts[index];
        }
        return None();
    }
}
