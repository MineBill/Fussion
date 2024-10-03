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
                .Binding = 0,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture{
                    .SampleType = GPU::TextureSampleType::Float{ true },
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 1,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Sampler{
                    .Type = GPU::SamplerBindingType::Filtering,
                },
                .Count = 1,
            }
        };

        GPU::BindGroupLayoutSpec spec{
            .Label = "SSAOBlur::BGL"sv,
            .Entries = entries,
        };

        m_bind_group_layout = Renderer::Device().CreateBindGroupLayout(spec);

        GPU::TextureSpec rt_spec{
            .Label = "SSAOBlur::RenderTarget"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { size.x, size.y, 1 },
            .Format = Format,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
        };
        m_render_target = Renderer::Device().CreateTexture(rt_spec);
        m_render_target.InitializeView();

        GPU::SamplerSpec sampler_spec{
            .label = "SSAOBlur::Sampler"sv,
            .AddressModeU = GPU::AddressMode::Repeat,
            .AddressModeV = GPU::AddressMode::Repeat,
            .AddressModeW = GPU::AddressMode::Repeat,
            .MagFilter = GPU::FilterMode::Nearest,
            .MinFilter = GPU::FilterMode::Nearest,
            .MipMapFilter = GPU::FilterMode::Linear,
        };

        m_sampler = Renderer::Device().CreateSampler(sampler_spec);

        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .Binding = 0,
                .Resource = m_render_target.View,
            },
            GPU::BindGroupEntry{
                .Binding = 1,
                .Resource = m_sampler,
            },
        };

        GPU::BindGroupSpec global_bg_spec{
            .Label = "SSAOBlur::BindGroup"sv,
            .Entries = bind_group_entries
        };

        m_bind_group = Renderer::Device().CreateBindGroup(m_bind_group_layout, global_bg_spec);

        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/blur.wgsl").Unwrap();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "SSAOBlur::Shader"sv,
            .Type = GPU::WGSLShader{
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts{
            m_bind_group_layout,
        };
        GPU::PipelineLayoutSpec pl_spec{
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        GPU::RenderPipelineSpec rp_spec{
            .Label = "SSAOBlur::RenderPipeline"sv,
            .Layout = layout,
            .Vertex = {},
            .Primitive = {
                .Topology = GPU::PrimitiveTopology::TriangleList,
                .StripIndexFormat = None(),
                .FrontFace = GPU::FrontFace::Ccw,
                .Cull = GPU::Face::None,
            },
            .DepthStencil = None(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = GPU::FragmentStage{
                .Targets = {
                    GPU::ColorTargetState{
                        .Format = Format,
                        .Blend = None(),
                        .WriteMask = GPU::ColorWrite::All,
                    },
                }
            },
        };

        m_pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
    }

    void SSAOBlur::resize(Vector2 const& new_size, GPU::Texture const& ssao_texture)
    {
        m_bind_group.Release();
        m_render_target.Release();

        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .Binding = 0,
                .Resource = ssao_texture.View,
            },
            GPU::BindGroupEntry{
                .Binding = 1,
                .Resource = m_sampler,
            },
        };

        GPU::BindGroupSpec bg_spec{
            .Label = "SSAOBlur::BindGroup"sv,
            .Entries = bind_group_entries
        };

        m_bind_group = Renderer::Device().CreateBindGroup(m_bind_group_layout, bg_spec);

        GPU::TextureSpec rt_spec{
            .Label = "SSAOBlur::RenderTarget"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { new_size.x, new_size.y, 1 },
            .Format = Format,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
        };
        m_render_target = Renderer::Device().CreateTexture(rt_spec);
    }

    void SSAOBlur::draw(GPU::CommandEncoder const& encoder, GPU::QuerySet const& set, u32 begin, u32 end)
    {
        ZoneScopedN("SSAO::Blur");

        using namespace GPU;
        std::array color_attachments{
            RenderPassColorAttachment{
                .View = m_render_target.View,
                .LoadOp = LoadOp::Clear,
                .StoreOp = StoreOp::Store,
                .ClearColor = Color::Indigo,
            },
        };

        RenderPassSpec spec{
            .Label = "SSAOBlur::RenderPass"sv,
            .ColorAttachments = color_attachments,
            .DepthStencilAttachment = None(),
            .TimestampWrites = RenderPassTimestampWrites{
                .Set = set,
                .BeginningOfPassWriteIndex = begin,
                .EndOfPassWriteIndex = end,
            }
        };
        auto rp = encoder.BeginRendering(spec);

        rp.SetPipeline(m_pipeline);
        rp.SetBindGroup(m_bind_group, 0);
        rp.Draw({ 0, 6 }, { 0, 1 });

        rp.End();
        rp.Release();
    }
}
