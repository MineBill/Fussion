#include "FussionPCH.h"
#include "HDRPipeline.h"

#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"

namespace Fussion {

    void HDRPipeline::Init(Vector2 size, GPU::TextureFormat output_format)
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
            .Label = "HDR::BGL"sv,
            .Entries = entries,
        };

        m_BindGroupLayout = Renderer::Device().CreateBindGroupLayout(spec);

        GPU::TextureSpec rt_spec{
            .Label = "HDR::RenderTarget"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { size.X, size.Y, 1 },
            .Format = Format,
            .MipLevelCount = 1,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
        };
        m_RenderTexture = Renderer::Device().CreateTexture(rt_spec);
        m_RenderTexture.InitializeView();

        GPU::SamplerSpec sampler_spec{
            .Label = "HDR::Sampler"sv,
            .AddressModeU = GPU::AddressMode::Repeat,
            .AddressModeV = GPU::AddressMode::Repeat,
            .AddressModeW = GPU::AddressMode::Repeat,
            .MagFilter = GPU::FilterMode::Linear,
            .MinFilter = GPU::FilterMode::Linear,
            .MipMapFilter = GPU::FilterMode::Linear,
        };

        m_Sampler = Renderer::Device().CreateSampler(sampler_spec);

        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .Binding = 0,
                .Resource = m_RenderTexture.View,
            },
            GPU::BindGroupEntry{
                .Binding = 1,
                .Resource = m_Sampler,
            },
        };

        GPU::BindGroupSpec global_bg_spec{
            .Label = "HDR::BindGroup"sv,
            .Entries = bind_group_entries
        };

        m_BindGroup = Renderer::Device().CreateBindGroup(m_BindGroupLayout, global_bg_spec);

        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/HDR.wgsl").Value();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "HDR::Shader"sv,
            .Type = GPU::WGSLShader{
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts{
            m_BindGroupLayout,
        };
        GPU::PipelineLayoutSpec pl_spec{
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        GPU::RenderPipelineSpec rp_spec{
            .Label = "HDR::RenderPipeline"sv,
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
            .Fragment = {
                .Targets = {
                    GPU::ColorTargetState{
                        .Format = output_format,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    },
                }
            },
        };

        m_Pipeline = Renderer::Device().CreateRenderPipeline(shader, rp_spec);
    }

    void HDRPipeline::Process(GPU::CommandEncoder& encoder, GPU::TextureView& output)
    {
        using namespace GPU;
        std::array color_attachments{
            RenderPassColorAttachment{
                .View = output,
                .LoadOp = LoadOp::Clear,
                .StoreOp = StoreOp::Store,
                .ClearColor = Color::Indigo,
            },
        };

        RenderPassSpec spec{
            .Label = "HDR::RenderPass",
            .ColorAttachments = color_attachments,
            .DepthStencilAttachment = None(),
        };
        auto rp = encoder.BeginRendering(spec);

        rp.SetPipeline(m_Pipeline);
        rp.SetBindGroup(m_BindGroup, 0);
        rp.Draw({ 0, 6 }, { 0, 1 });

        rp.End();
        rp.Release();
    }

    void HDRPipeline::Resize(Vector2 size)
    {
        m_RenderTexture.Release();
        GPU::TextureSpec rt_spec{
            .Label = "HDR::RenderTarget"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { size.X, size.Y, 1 },
            .Format = Format,
            .MipLevelCount = 1,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
        };
        m_RenderTexture = Renderer::Device().CreateTexture(rt_spec);
        m_RenderTexture.InitializeView();

        m_BindGroup.Release();
        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .Binding = 0,
                .Resource = m_RenderTexture.View,
            },
            GPU::BindGroupEntry{
                .Binding = 1,
                .Resource = m_Sampler,
            },
        };

        GPU::BindGroupSpec global_bg_spec{
            .Label = "HDR::BindGroup"sv,
            .Entries = bind_group_entries
        };

        m_BindGroup = Renderer::Device().CreateBindGroup(m_BindGroupLayout, global_bg_spec);
    }

    auto HDRPipeline::View() -> GPU::TextureView&
    {
        return m_RenderTexture.View;
    }
}
