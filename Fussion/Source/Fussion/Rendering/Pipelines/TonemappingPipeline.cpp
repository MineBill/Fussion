#include "FussionPCH.h"
#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"
#include "TonemappingPipeline.h"

namespace Fussion {

    void TonemappingPipeline::init(Vector2 size, GPU::TextureFormat output_format)
    {
        std::array entries {
            GPU::BindGroupLayoutEntry {
                .Binding = 0,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture {
                    .SampleType = GPU::TextureSampleType::Float { true },
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 1,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Sampler {
                    .Type = GPU::SamplerBindingType::Filtering,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 2,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Buffer {},
                .Count = 1,
            }
        };

        GPU::BindGroupLayoutSpec spec {
            .Label = "HDR::BGL"sv,
            .Entries = entries,
        };

        m_bind_group_layout = Renderer::Device().CreateBindGroupLayout(spec);

        GPU::SamplerSpec sampler_spec {
            .label = "HDR::Sampler"sv,
            .AddressModeU = GPU::AddressMode::Repeat,
            .AddressModeV = GPU::AddressMode::Repeat,
            .AddressModeW = GPU::AddressMode::Repeat,
            .MagFilter = GPU::FilterMode::Linear,
            .MinFilter = GPU::FilterMode::Linear,
            .MipMapFilter = GPU::FilterMode::Linear,
        };

        m_sampler = Renderer::Device().CreateSampler(sampler_spec);

        m_tonemapping_buffer = UniformBuffer<PostProcessing::Tonemapping>::Create(Renderer::Device(), "Tonemapping Settings Buffer"sv);
        resize(size);

        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/HDR.wgsl").Unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .Label = "HDR::Shader"sv,
            .Type = GPU::WGSLShader {
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts {
            m_bind_group_layout,
        };
        GPU::PipelineLayoutSpec pl_spec {
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        GPU::RenderPipelineSpec rp_spec {
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
            .Fragment = GPU::FragmentStage {
                .Targets = {
                    GPU::ColorTargetState {
                        .Format = output_format,
                        .Blend = None(),
                        .WriteMask = GPU::ColorWrite::All,
                    },
                },
            },
        };

        m_pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
    }

    void TonemappingPipeline::process(GPU::CommandEncoder& encoder, GPU::TextureView& output, RenderContext const& render_context)
    {
        using namespace GPU;
        std::array color_attachments {
            RenderPassColorAttachment {
                .View = output,
                .LoadOp = LoadOp::Clear,
                .StoreOp = StoreOp::Store,
                .ClearColor = Color::Indigo,
            },
        };

        RenderPassSpec spec {
            .Label = "HDR::RenderPass"sv,
            .ColorAttachments = color_attachments,
            .DepthStencilAttachment = None(),
        };
        auto rp = encoder.BeginRendering(spec);

        m_tonemapping_buffer.Data = render_context.PostProcessingSettings.TonemappingSettings;
        m_tonemapping_buffer.flush();

        rp.SetPipeline(m_pipeline);
        rp.SetBindGroup(m_bind_group, 0);
        rp.Draw({ 0, 4 }, { 0, 1 });

        rp.End();
        rp.Release();
    }

    void TonemappingPipeline::resize(Vector2 size)
    {
        m_render_texture.Release();
        GPU::TextureSpec rt_spec {
            .Label = "HDR::RenderTarget"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { size.x, size.y, 1 },
            .Format = Format,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
        };
        m_render_texture = Renderer::Device().CreateTexture(rt_spec);

        m_bind_group.Release();
        std::array bind_group_entries {
            GPU::BindGroupEntry {
                .Binding = 0,
                .Resource = m_render_texture.View,
            },
            GPU::BindGroupEntry {
                .Binding = 1,
                .Resource = m_sampler,
            },
            GPU::BindGroupEntry {
                .Binding = 2,
                // TODO: Make the UniformBuffer create the binding directly?
                .Resource = GPU::BufferBinding {
                    .TargetBuffer = m_tonemapping_buffer.Buffer(),
                    .Offset = 0,
                    .Size = m_tonemapping_buffer.Buffer().Size() },
            },
        };

        GPU::BindGroupSpec global_bg_spec {
            .Label = "HDR::BindGroup"sv,
            .Entries = bind_group_entries
        };

        m_bind_group = Renderer::Device().CreateBindGroup(m_bind_group_layout, global_bg_spec);
    }

    auto TonemappingPipeline::view() -> GPU::TextureView&
    {
        return m_render_texture.View;
    }
}
