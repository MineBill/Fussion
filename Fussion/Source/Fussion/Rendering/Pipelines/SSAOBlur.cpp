#include "FussionPCH.h"
#include "SSAOBlur.h"

#include "Assets/AssetManager.h"
#include "Assets/ShaderAsset.h"
#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"

#include <tracy/Tracy.hpp>

namespace Fussion {
    void SSAOBlur::init(Vector2 const& size)
    {
        // std::array entries {
        //     GPU::BindGroupLayoutEntry {
        //         .Binding = 0,
        //         .Visibility = GPU::ShaderStage::Fragment,
        //         .Type = GPU::BindingType::Texture {
        //             .SampleType = GPU::TextureSampleType::Float { true },
        //             .ViewDimension = GPU::TextureViewDimension::D2,
        //             .MultiSampled = false,
        //         },
        //         .Count = 1,
        //     },
        //     GPU::BindGroupLayoutEntry {
        //         .Binding = 1,
        //         .Visibility = GPU::ShaderStage::Fragment,
        //         .Type = GPU::BindingType::Sampler {
        //             .Type = GPU::SamplerBindingType::Filtering,
        //         },
        //         .Count = 1,
        //     }
        // };
        //
        // GPU::BindGroupLayoutSpec spec {
        //     .Label = "SSAOBlur::BGL"sv,
        //     .Entries = entries,
        // };
        //
        // m_BindGroupLayout = Renderer::Device().CreateBindGroupLayout(spec);

        constexpr auto path = "Assets/Shaders/Slang/Effects/Blur.slang";
        auto compiledShader = GPU::ShaderProcessor::compile_slang(path).unwrap();
        compiledShader.metadata.use_depth = false;
        auto shader = make_ref<ShaderAsset>(compiledShader, std::vector { Format });
        m_shader = AssetManager::create_virtual_asset_ref_with_path<ShaderAsset>(shader, path);

        GPU::TextureSpec rt_spec {
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

        GPU::SamplerSpec sampler_spec {
            .label = "SSAOBlur::Sampler"sv,
            .address_mode_u = GPU::AddressMode::Repeat,
            .address_mode_v = GPU::AddressMode::Repeat,
            .address_mode_w = GPU::AddressMode::Repeat,
            .mag_filter = GPU::FilterMode::Nearest,
            .min_filter = GPU::FilterMode::Nearest,
            .mip_map_filter = GPU::FilterMode::Linear,
        };

        m_sampler = Renderer::device().create_sampler(sampler_spec);

        std::array bind_group_entries {
            GPU::BindGroupEntry {
                .binding = 0,
                .resource = m_render_target.view,
            },
            GPU::BindGroupEntry {
                .binding = 1,
                .resource = m_sampler,
            },
        };

        GPU::BindGroupSpec bgSpec {
            .label = "SSAOBlur::BindGroup"sv,
            .entries = bind_group_entries
        };

        m_bind_group = Renderer::device().create_bind_group(shader->get_bind_group_layout_for(0).unwrap(), bgSpec);
    }

    void SSAOBlur::resize(Vector2 const& new_size, GPU::Texture const& ssao_texture)
    {
        m_bind_group.release();
        m_render_target.release();

        std::array bgEntries {
            GPU::BindGroupEntry {
                .binding = 0,
                .resource = ssao_texture.view,
            },
            GPU::BindGroupEntry {
                .binding = 1,
                .resource = m_sampler,
            },
        };

        GPU::BindGroupSpec bg_spec {
            .label = "SSAOBlur::BindGroup"sv,
            .entries = bgEntries
        };

        auto shader = m_shader.get();
        m_bind_group = Renderer::device().create_bind_group(shader->get_bind_group_layout_for(0).unwrap(), bg_spec);

        GPU::TextureSpec rt_spec {
            .label = "SSAOBlur::RenderTarget"sv,
            .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .dimension = GPU::TextureDimension::D2,
            .size = { new_size.x, new_size.y, 1 },
            .format = Format,
            .sample_count = 1,
            .aspect = GPU::TextureAspect::All,
        };
        m_render_target = Renderer::device().create_texture(rt_spec);
    }

    void SSAOBlur::render(GPU::CommandEncoder const& encoder, GPU::QuerySet const& set, u32 begin, u32 end)
    {
        ZoneScopedN("SSAO::Blur");

        using namespace GPU;
        std::array color_attachments {
            RenderPassColorAttachment {
                .view = m_render_target.view,
                .load_op = LoadOp::Clear,
                .store_op = StoreOp::Store,
                .clear_color = Color::Indigo,
            },
        };

        RenderPassSpec spec {
            .label = "SSAOBlur::RenderPass"sv,
            .color_attachments = color_attachments,
            .depth_stencil_attachment = None(),
            .timestamp_writes = RenderPassTimestampWrites {
                .set = set,
                .beginning_of_pass_write_index = begin,
                .end_of_pass_write_index = end,
            }
        };
        auto rp = encoder.begin_rendering(spec);

        auto shader = m_shader.get();
        rp.set_pipeline(shader->pipeline());
        rp.set_bind_group(m_bind_group, 0);
        rp.draw({ 0, 6 }, { 0, 1 });

        rp.end();
        rp.release();
    }
}
