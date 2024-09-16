#include "W:\source\projects\Fussion\build\.gens\Fussion\windows\x64\debug\Fussion\Source\FussionPCH.h"
#include "SSAOBlur.h"

#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"

#include <tracy/Tracy.hpp>

namespace Fussion {
    void SSAOBlur::init(Vector2 const& size)
    {
        std::array entries{
            GPU::BindGroupLayoutEntry{
                .binding = 0,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture{
                    .sample_type = GPU::TextureSampleType::Float{ true },
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 1,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Sampler{
                    .type = GPU::SamplerBindingType::Filtering,
                },
                .count = 1,
            }
        };

        GPU::BindGroupLayoutSpec spec{
            .label = "SSAOBlur::BGL"sv,
            .entries = entries,
        };

        m_bind_group_layout = Renderer::device().create_bind_group_layout(spec);

        GPU::TextureSpec rt_spec{
            .label = "SSAOBlur::RenderTarget"sv,
            .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .dimension = GPU::TextureDimension::D2,
            .size = { size.x, size.y, 1 },
            .format = Format,
            .sample_count = 1,
            .aspect = GPU::TextureAspect::All,
        };
        m_render_target = Renderer::device().create_texture(rt_spec);
        m_render_target.initialize_view();

        GPU::SamplerSpec sampler_spec{
            .label = "SSAOBlur::Sampler"sv,
            .address_mode_u = GPU::AddressMode::Repeat,
            .address_mode_v = GPU::AddressMode::Repeat,
            .address_mode_w = GPU::AddressMode::Repeat,
            .mag_filter = GPU::FilterMode::Nearest,
            .min_filter = GPU::FilterMode::Nearest,
            .mip_map_filter = GPU::FilterMode::Linear,
        };

        m_sampler = Renderer::device().create_sampler(sampler_spec);

        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .binding = 0,
                .resource = m_render_target.view,
            },
            GPU::BindGroupEntry{
                .binding = 1,
                .resource = m_sampler,
            },
        };

        GPU::BindGroupSpec global_bg_spec{
            .label = "SSAOBlur::BindGroup"sv,
            .entries = bind_group_entries
        };

        m_bind_group = Renderer::device().create_bind_group(m_bind_group_layout, global_bg_spec);

        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/blur.wgsl").value();

        GPU::ShaderModuleSpec shader_spec{
            .label = "SSAOBlur::Shader"sv,
            .type = GPU::WGSLShader{
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts{
            m_bind_group_layout,
        };
        GPU::PipelineLayoutSpec pl_spec{
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        GPU::RenderPipelineSpec rp_spec{
            .label = "SSAOBlur::RenderPipeline"sv,
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
            .fragment = GPU::FragmentStage{
                .targets = {
                    GPU::ColorTargetState{
                        .format = Format,
                        .blend = None(),
                        .write_mask = GPU::ColorWrite::All,
                    },
                }
            },
        };

        m_pipeline = Renderer::device().create_render_pipeline(shader, rp_spec);
    }

    void SSAOBlur::resize(Vector2 const& new_size, GPU::Texture const& ssao_texture)
    {
        m_bind_group.release();
        m_render_target.release();

        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .binding = 0,
                .resource = ssao_texture.view,
            },
            GPU::BindGroupEntry{
                .binding = 1,
                .resource = m_sampler,
            },
        };

        GPU::BindGroupSpec bg_spec{
            .label = "SSAOBlur::BindGroup"sv,
            .entries = bind_group_entries
        };

        m_bind_group = Renderer::device().create_bind_group(m_bind_group_layout, bg_spec);

        GPU::TextureSpec rt_spec{
            .label = "SSAOBlur::RenderTarget"sv,
            .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .dimension = GPU::TextureDimension::D2,
            .size = { new_size.x, new_size.y, 1 },
            .format = Format,
            .sample_count = 1,
            .aspect = GPU::TextureAspect::All,
        };
        m_render_target = Renderer::device().create_texture(rt_spec);
        m_render_target.initialize_view();
    }

    void SSAOBlur::draw(GPU::CommandEncoder const& encoder)
    {
        ZoneScopedN("SSAO::Blur");

        using namespace GPU;
        std::array color_attachments{
            RenderPassColorAttachment{
                .view = m_render_target.view,
                .load_op = LoadOp::Clear,
                .store_op = StoreOp::Store,
                .clear_color = Color::Indigo,
            },
        };

        RenderPassSpec spec{
            .label = "SSAOBlur::RenderPass"sv,
            .color_attachments = color_attachments,
            .depth_stencil_attachment = None(),
        };
        auto rp = encoder.begin_rendering(spec);

        rp.set_pipeline(m_pipeline);
        rp.set_bind_group(m_bind_group, 0);
        rp.draw({ 0, 6 }, { 0, 1 });

        rp.end();
        rp.release();
    }
}
