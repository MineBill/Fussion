#include "FussionPCH.h"
#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"
#include "TonemappingPipeline.h"

namespace Fussion {

    void TonemappingPipeline::init(Vector2 size, GPU::TextureFormat output_format)
    {
        std::array entries {
            GPU::BindGroupLayoutEntry {
                .binding = 0,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture {
                    .sample_type = GPU::TextureSampleType::Float { true },
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 1,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Sampler {
                    .type = GPU::SamplerBindingType::Filtering,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 2,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Buffer {},
                .count = 1,
            }
        };

        GPU::BindGroupLayoutSpec spec {
            .label = "HDR::BGL"sv,
            .entries = entries,
        };

        m_bind_group_layout = Renderer::device().create_bind_group_layout(spec);

        GPU::SamplerSpec sampler_spec {
            .label = "HDR::Sampler"sv,
            .address_mode_u = GPU::AddressMode::Repeat,
            .address_mode_v = GPU::AddressMode::Repeat,
            .address_mode_w = GPU::AddressMode::Repeat,
            .mag_filter = GPU::FilterMode::Linear,
            .min_filter = GPU::FilterMode::Linear,
            .mip_map_filter = GPU::FilterMode::Linear,
        };

        m_sampler = Renderer::device().create_sampler(sampler_spec);

        m_tonemapping_buffer = UniformBuffer<PostProcessing::TonemappingSettings>::create(Renderer::device(), "Tonemapping Settings Buffer"sv);
        resize(size);

        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/HDR.wgsl").unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .label = "HDR::Shader"sv,
            .type = GPU::WGSLShader {
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts {
            m_bind_group_layout,
        };
        GPU::PipelineLayoutSpec pl_spec {
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        GPU::RenderPipelineSpec rp_spec {
            .label = "HDR::RenderPipeline"sv,
            .layout = layout,
            .vertex = {},
            .primitive = {
                .topology = GPU::PrimitiveTopology::TriangleList,
                .strip_index_format = None(),
                .front_face = GPU::FrontFace::Ccw,
                .cull = GPU::Face::None,
            },
            .depth_stencil = None(),
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = GPU::FragmentStage {
                .targets = {
                    GPU::ColorTargetState {
                        .format = output_format,
                        .blend = None(),
                        .write_mask = GPU::ColorWrite::All,
                    },
                },
            },
        };

        m_pipeline = Renderer::device().create_render_pipeline(shader, shader, rp_spec);
    }

    void TonemappingPipeline::process(GPU::CommandEncoder& encoder, GPU::TextureView& output, RenderContext const& render_context)
    {
        using namespace GPU;
        std::array color_attachments {
            RenderPassColorAttachment {
                .view = output,
                .load_op = LoadOp::Clear,
                .store_op = StoreOp::Store,
                .clear_color = Color::Indigo,
            },
        };

        RenderPassSpec spec {
            .label = "HDR::RenderPass"sv,
            .color_attachments = color_attachments,
            .depth_stencil_attachment = None(),
        };
        auto rp = encoder.begin_rendering(spec);

        m_tonemapping_buffer.data = render_context.post_processing.tonemapping_settings;
        m_tonemapping_buffer.flush();

        rp.set_pipeline(m_pipeline);
        rp.set_bind_group(m_bind_group, 0);
        rp.draw({ 0, 4 }, { 0, 1 });

        rp.end();
        rp.release();
    }

    void TonemappingPipeline::resize(Vector2 size)
    {
        m_render_texture.release();
        GPU::TextureSpec rt_spec {
            .label = "HDR::RenderTarget"sv,
            .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .dimension = GPU::TextureDimension::D2,
            .size = { size.x, size.y, 1 },
            .format = Format,
            .sample_count = 1,
            .aspect = GPU::TextureAspect::All,
        };
        m_render_texture = Renderer::device().create_texture(rt_spec);

        m_bind_group.release();
        std::array bind_group_entries {
            GPU::BindGroupEntry {
                .binding = 0,
                .resource = m_render_texture.view,
            },
            GPU::BindGroupEntry {
                .binding = 1,
                .resource = m_sampler,
            },
            GPU::BindGroupEntry {
                .binding = 2,
                // TODO: Make the UniformBuffer create the binding directly?
                .resource = GPU::BufferBinding {
                    .buffer = m_tonemapping_buffer.buffer(),
                    .offset = 0,
                    .size = m_tonemapping_buffer.buffer().size() },
            },
        };

        GPU::BindGroupSpec global_bg_spec {
            .label = "HDR::BindGroup"sv,
            .entries = bind_group_entries
        };

        m_bind_group = Renderer::device().create_bind_group(m_bind_group_layout, global_bg_spec);
    }

    auto TonemappingPipeline::view() -> GPU::TextureView&
    {
        return m_render_texture.view;
    }
}
