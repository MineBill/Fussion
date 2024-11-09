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
        auto compiledShader = GPU::ShaderProcessor::CompileSlang(path).unwrap();
        compiledShader.Metadata.UseDepth = false;
        auto shader = make_ref<ShaderAsset>(compiledShader, std::vector { Format });
        m_shader = AssetManager::create_virtual_asset_ref_with_path<ShaderAsset>(shader, path);

        GPU::TextureSpec rt_spec {
            .Label = "SSAOBlur::RenderTarget"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { size.x, size.y, 1 },
            .Format = Format,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
        };
        m_render_target = Renderer::device().CreateTexture(rt_spec);
        m_render_target.InitializeView();

        GPU::SamplerSpec sampler_spec {
            .label = "SSAOBlur::Sampler"sv,
            .AddressModeU = GPU::AddressMode::Repeat,
            .AddressModeV = GPU::AddressMode::Repeat,
            .AddressModeW = GPU::AddressMode::Repeat,
            .MagFilter = GPU::FilterMode::Nearest,
            .MinFilter = GPU::FilterMode::Nearest,
            .MipMapFilter = GPU::FilterMode::Linear,
        };

        m_sampler = Renderer::device().CreateSampler(sampler_spec);

        std::array bind_group_entries {
            GPU::BindGroupEntry {
                .Binding = 0,
                .Resource = m_render_target.View,
            },
            GPU::BindGroupEntry {
                .Binding = 1,
                .Resource = m_sampler,
            },
        };

        GPU::BindGroupSpec bgSpec {
            .Label = "SSAOBlur::BindGroup"sv,
            .Entries = bind_group_entries
        };

        m_bind_group = Renderer::device().CreateBindGroup(shader->get_bind_group_layout_for(0).unwrap(), bgSpec);
    }

    void SSAOBlur::resize(Vector2 const& new_size, GPU::Texture const& ssao_texture)
    {
        m_bind_group.Release();
        m_render_target.Release();

        std::array bgEntries {
            GPU::BindGroupEntry {
                .Binding = 0,
                .Resource = ssao_texture.View,
            },
            GPU::BindGroupEntry {
                .Binding = 1,
                .Resource = m_sampler,
            },
        };

        GPU::BindGroupSpec bg_spec {
            .Label = "SSAOBlur::BindGroup"sv,
            .Entries = bgEntries
        };

        auto shader = m_shader.get();
        m_bind_group = Renderer::device().CreateBindGroup(shader->get_bind_group_layout_for(0).unwrap(), bg_spec);

        GPU::TextureSpec rt_spec {
            .Label = "SSAOBlur::RenderTarget"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { new_size.x, new_size.y, 1 },
            .Format = Format,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
        };
        m_render_target = Renderer::device().CreateTexture(rt_spec);
    }

    void SSAOBlur::render(GPU::CommandEncoder const& encoder, GPU::QuerySet const& set, u32 begin, u32 end)
    {
        ZoneScopedN("SSAO::Blur");

        using namespace GPU;
        std::array color_attachments {
            RenderPassColorAttachment {
                .View = m_render_target.View,
                .LoadOp = LoadOp::Clear,
                .StoreOp = StoreOp::Store,
                .ClearColor = Color::Indigo,
            },
        };

        RenderPassSpec spec {
            .Label = "SSAOBlur::RenderPass"sv,
            .ColorAttachments = color_attachments,
            .DepthStencilAttachment = None(),
            .TimestampWrites = RenderPassTimestampWrites {
                .Set = set,
                .BeginningOfPassWriteIndex = begin,
                .EndOfPassWriteIndex = end,
            }
        };
        auto rp = encoder.BeginRendering(spec);

        auto shader = m_shader.get();
        rp.SetPipeline(shader->pipeline());
        rp.SetBindGroup(m_bind_group, 0);
        rp.Draw({ 0, 6 }, { 0, 1 });

        rp.End();
        rp.Release();
    }
}
