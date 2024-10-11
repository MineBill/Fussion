#include "FussionPCH.h"
#include "SSAOBlur.h"

#include "Assets/AssetManager.h"
#include "Assets/ShaderAsset.h"
#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"

#include <tracy/Tracy.hpp>

namespace Fussion {
    void SSAOBlur::Init(Vector2 const& size)
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
        auto compiledShader = GPU::ShaderProcessor::CompileSlang(path).Unwrap();
        compiledShader.Metadata.UseDepth = false;
        auto shader = MakeRef<ShaderAsset>(compiledShader, std::vector { Format });
        m_Shader = AssetManager::CreateVirtualAssetRefWithPath<ShaderAsset>(shader, path);

        GPU::TextureSpec rt_spec {
            .Label = "SSAOBlur::RenderTarget"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { size.x, size.y, 1 },
            .Format = Format,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
        };
        m_RenderTarget = Renderer::Device().CreateTexture(rt_spec);
        m_RenderTarget.InitializeView();

        GPU::SamplerSpec sampler_spec {
            .label = "SSAOBlur::Sampler"sv,
            .AddressModeU = GPU::AddressMode::Repeat,
            .AddressModeV = GPU::AddressMode::Repeat,
            .AddressModeW = GPU::AddressMode::Repeat,
            .MagFilter = GPU::FilterMode::Nearest,
            .MinFilter = GPU::FilterMode::Nearest,
            .MipMapFilter = GPU::FilterMode::Linear,
        };

        m_Sampler = Renderer::Device().CreateSampler(sampler_spec);

        std::array bind_group_entries {
            GPU::BindGroupEntry {
                .Binding = 0,
                .Resource = m_RenderTarget.View,
            },
            GPU::BindGroupEntry {
                .Binding = 1,
                .Resource = m_Sampler,
            },
        };

        GPU::BindGroupSpec bgSpec {
            .Label = "SSAOBlur::BindGroup"sv,
            .Entries = bind_group_entries
        };

        m_BindGroup = Renderer::Device().CreateBindGroup(shader->GetBindGroupLayout(0).Unwrap(), bgSpec);

        // auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/blur.wgsl").Unwrap();
        //
        // GPU::ShaderModuleSpec shader_spec {
        //     .Label = "SSAOBlur::Shader"sv,
        //     .Type = GPU::WGSLShader {
        //         .Source = shader_src,
        //     },
        //     .VertexEntryPoint = "vs_main",
        //     .FragmentEntryPoint = "fs_main",
        // };
        //
        // auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        // std::array bind_group_layouts {
        //     m_BindGroupLayout,
        // };
        // GPU::PipelineLayoutSpec pl_spec {
        //     .BindGroupLayouts = bind_group_layouts
        // };
        // auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);
        //
        // GPU::RenderPipelineSpec rp_spec {
        //     .Label = "SSAOBlur::RenderPipeline"sv,
        //     .Layout = layout,
        //     .Vertex = {},
        //     .Primitive = {
        //         .Topology = GPU::PrimitiveTopology::TriangleList,
        //         .StripIndexFormat = None(),
        //         .FrontFace = GPU::FrontFace::Ccw,
        //         .Cull = GPU::Face::None,
        //     },
        //     .DepthStencil = None(),
        //     .MultiSample = GPU::MultiSampleState::Default(),
        //     .Fragment = GPU::FragmentStage { .Targets = {
        //                                          GPU::ColorTargetState {
        //                                              .Format = Format,
        //                                              .Blend = None(),
        //                                              .WriteMask = GPU::ColorWrite::All,
        //                                          },
        //                                      } },
        // };
        //
        // m_Pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
    }

    void SSAOBlur::Resize(Vector2 const& new_size, GPU::Texture const& ssao_texture)
    {
        m_BindGroup.Release();
        m_RenderTarget.Release();

        std::array bgEntries {
            GPU::BindGroupEntry {
                .Binding = 0,
                .Resource = ssao_texture.View,
            },
            GPU::BindGroupEntry {
                .Binding = 1,
                .Resource = m_Sampler,
            },
        };

        GPU::BindGroupSpec bg_spec {
            .Label = "SSAOBlur::BindGroup"sv,
            .Entries = bgEntries
        };

        auto shader = m_Shader.Get();
        m_BindGroup = Renderer::Device().CreateBindGroup(shader->GetBindGroupLayout(0).Unwrap(), bg_spec);

        GPU::TextureSpec rt_spec {
            .Label = "SSAOBlur::RenderTarget"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureDimension::D2,
            .Size = { new_size.x, new_size.y, 1 },
            .Format = Format,
            .SampleCount = 1,
            .Aspect = GPU::TextureAspect::All,
        };
        m_RenderTarget = Renderer::Device().CreateTexture(rt_spec);
    }

    void SSAOBlur::Render(GPU::CommandEncoder const& encoder, GPU::QuerySet const& set, u32 begin, u32 end)
    {
        ZoneScopedN("SSAO::Blur");

        using namespace GPU;
        std::array color_attachments {
            RenderPassColorAttachment {
                .View = m_RenderTarget.View,
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

        auto shader = m_Shader.Get();
        rp.SetPipeline(shader->Pipeline());
        rp.SetBindGroup(m_BindGroup, 0);
        rp.Draw({ 0, 6 }, { 0, 1 });

        rp.End();
        rp.Release();
    }
}
