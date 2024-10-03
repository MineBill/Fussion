#include "SceneRenderer.h"

#include "EditorPCH.h"
#include "Fussion/Core/Mem.h"
#include "Fussion/Rendering/Pipelines/IBLIrradiance.h"
#include "Project/Project.h"

#include <Fussion/Core/Application.h>
#include <Fussion/Core/Time.h>
#include <Fussion/Debug/Debug.h>
#include <Fussion/GPU/ShaderProcessor.h>
#include <Fussion/OS/FileSystem.h>
#include <Fussion/Rendering/Renderer.h>
#include <magic_enum/magic_enum.hpp>
#include <random>
#include <tracy/Tracy.hpp>

#undef far
#undef near
#undef min
#undef max

using namespace Fussion;

constexpr GPU::TextureFormat SSAO_NOISE_TEXTURE_FORMAT = GPU::TextureFormat::RGBA32Float;

void begin_pipeline_statistics_query(GPU::RenderPassEncoder const& encoder, GPU::QuerySet const& set, u32 index)
{
    if (Renderer::HasPipelineStatistics()) {
        encoder.BeginPipelineStatisticsQuery(set, index);
    }
}

void end_pipeline_statistics_query(GPU::RenderPassEncoder const& encoder)
{
    if (Renderer::HasPipelineStatistics()) {
        encoder.EndPipelineStatisticsQuery();
    }
}

void GBuffer::init(Vector2 const& size, GPU::BindGroupLayout const& global_bind_group_layout)
{
    GPU::TextureSpec spec {
        .Label = "GBuffer::position"sv,
        .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3 { size, 1 },
        .Format = GPU::TextureFormat::RGBA16Float,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::All,
    };
    rt_position = Renderer::Device().CreateTexture(spec);

    spec.Label = "GBuffer::normal"sv;
    rt_normal = Renderer::Device().CreateTexture(spec);

    spec.Label = "GBuffer::albedo"sv;
    rt_albedo = Renderer::Device().CreateTexture(spec);

    auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/GBuffer.wgsl").Unwrap();

    GPU::ShaderModuleSpec shader_spec {
        .Label = "GBuffer::shader"sv,
        .Type = GPU::WGSLShader {
            .Source = shader_src,
        },
        .VertexEntryPoint = "vs_main",
        .FragmentEntryPoint = "fs_main",
    };

    auto shader = Renderer::Device().CreateShaderModule(shader_spec);

    std::array entries {
        GPU::BindGroupLayoutEntry {
            .Binding = 0,
            .Visibility = GPU::ShaderStage::Vertex,
            .Type = GPU::BindingType::Buffer {
                .Type = GPU::BufferBindingType::Storage {
                    .ReadOnly = true,
                },
                .HasDynamicOffset = false,
                .MinBindingSize = None(),
            },
            .Count = 1,
        },
    };

    GPU::BindGroupLayoutSpec bgl_spec {
        .Label = "GBuffer::bind_group_layout"sv,
        .Entries = entries,
    };

    bind_group_layout = Renderer::Device().CreateBindGroupLayout(bgl_spec);

    std::array bind_group_layouts {
        global_bind_group_layout,
        bind_group_layout,
    };
    GPU::PipelineLayoutSpec pl_spec {
        .BindGroupLayouts = bind_group_layouts
    };
    auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

    std::array attributes {
        GPU::VertexAttribute {
            .Type = GPU::ElementType::Float3,
            .ShaderLocation = 0,
        },
        GPU::VertexAttribute {
            .Type = GPU::ElementType::Float3,
            .ShaderLocation = 1,
        },
        GPU::VertexAttribute {
            .Type = GPU::ElementType::Float4,
            .ShaderLocation = 2,
        },
        GPU::VertexAttribute {
            .Type = GPU::ElementType::Float2,
            .ShaderLocation = 3,
        },
        GPU::VertexAttribute {
            .Type = GPU::ElementType::Float3,
            .ShaderLocation = 4,
        },
    };
    auto attribute_layout = GPU::VertexBufferLayout::Create(attributes);

    GPU::RenderPipelineSpec rp_spec {
        .Label = "GBuffer::pipeline"sv,
        .Layout = layout,
        .Vertex = {
            .AttributeLayouts = { attribute_layout },
        },
        .Primitive = GPU::PrimitiveState::Default(),
        .DepthStencil = GPU::DepthStencilState::Default(),
        .MultiSample = GPU::MultiSampleState::Default(),
        .Fragment = GPU::FragmentStage {
            .Targets = {
                GPU::ColorTargetState {
                    .Format = GPU::TextureFormat::RGBA16Float,
                    .Blend = GPU::BlendState::Default(),
                    .WriteMask = GPU::ColorWrite::All,
                },
                GPU::ColorTargetState {
                    .Format = GPU::TextureFormat::RGBA16Float,
                    .Blend = GPU::BlendState::Default(),
                    .WriteMask = GPU::ColorWrite::All,
                },
                GPU::ColorTargetState {
                    .Format = GPU::TextureFormat::RGBA16Float,
                    .Blend = GPU::BlendState::Default(),
                    .WriteMask = GPU::ColorWrite::All,
                },
            },
        },
    };

    pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
}

void GBuffer::resize(Vector2 const& new_size)
{
    rt_position.Release();

    rt_normal.Release();

    rt_albedo.Release();

    GPU::TextureSpec spec {
        .Label = "GBuffer::position"sv,
        .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3 { new_size, 1 },
        .Format = GPU::TextureFormat::RGBA16Float,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::All,
    };
    rt_position = Renderer::Device().CreateTexture(spec);

    spec.Label = "GBuffer::normal"sv;
    rt_normal = Renderer::Device().CreateTexture(spec);

    spec.Label = "GBuffer::albedo"sv;
    rt_albedo = Renderer::Device().CreateTexture(spec);
}

void GBuffer::do_pass(GPU::CommandEncoder& encoder)
{
    (void)encoder;
}

void SSAO::init(Vector2 const& size, GBuffer const& gbuffer, GPU::BindGroupLayout const& global_bind_group_layout)
{
    GPU::TextureSpec spec {
        .Label = "SSAO::render_target"sv,
        .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3 { size, 1 },
        .Format = GPU::TextureFormat::R32Float,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::All,
    };
    render_target = Renderer::Device().CreateTexture(spec);

    GPU::TextureSpec noise_spec {
        .Label = "SSAO::noise_texture"sv,
        .Usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3 { 4, 4, 1 },
        .Format = SSAO_NOISE_TEXTURE_FORMAT,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::All,
    };
    noise_texture = Renderer::Device().CreateTexture(noise_spec);

    auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/SSAO.wgsl").Unwrap();

    GPU::ShaderModuleSpec shader_spec {
        .Label = "SSAO::shader"sv,
        .Type = GPU::WGSLShader {
            .Source = shader_src,
        },
        .VertexEntryPoint = "vs_main",
        .FragmentEntryPoint = "fs_main",
    };

    auto shader = Renderer::Device().CreateShaderModule(shader_spec);

    std::array entries {
        GPU::BindGroupLayoutEntry {
            .Binding = 0,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Texture {
                .SampleType = GPU::TextureSampleType::Float {},
                .ViewDimension = GPU::TextureViewDimension::D2,
            },
            .Count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .Binding = 1,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Texture {
                .SampleType = GPU::TextureSampleType::Float {},
                .ViewDimension = GPU::TextureViewDimension::D2,
            },
            .Count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .Binding = 2,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Texture {
                .SampleType = GPU::TextureSampleType::Float {
                    .Filterable = true,
                },
                .ViewDimension = GPU::TextureViewDimension::D2,
            },
            .Count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .Binding = 3,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Sampler { .Type = GPU::SamplerBindingType::Filtering },
            .Count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .Binding = 4,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Sampler { .Type = GPU::SamplerBindingType::Filtering },
            .Count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .Binding = 5,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Buffer {
                .Type = GPU::BufferBindingType::Storage {
                    .ReadOnly = true,
                },
                .HasDynamicOffset = false,
                .MinBindingSize = None(),
            },
            .Count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .Binding = 6,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Buffer {
                .Type = GPU::BufferBindingType::Uniform {},
                .HasDynamicOffset = false,
                .MinBindingSize = None(),
            },
            .Count = 1,
        },
    };

    GPU::BindGroupLayoutSpec bgl_spec {
        .Label = "SSAO::bind_group_layout"sv,
        .Entries = entries,
    };

    bind_group_layout = Renderer::Device().CreateBindGroupLayout(bgl_spec);

    GPU::SamplerSpec sampler_spec {};
    sampler_spec.label = "SSAO::sampler"sv;
    sampler_spec.AddressModeU = GPU::AddressMode::ClampToEdge;
    sampler_spec.AddressModeW = GPU::AddressMode::ClampToEdge;
    sampler = Renderer::Device().CreateSampler(sampler_spec);

    sampler_spec = GPU::SamplerSpec {};
    sampler_spec.label = "SSAO::noise_sampler"sv,
    sampler_spec.AddressModeU = GPU::AddressMode::Repeat;
    sampler_spec.AddressModeV = GPU::AddressMode::Repeat;
    sampler_spec.AddressModeW = GPU::AddressMode::Repeat;
    noise_sampler = Renderer::Device().CreateSampler(sampler_spec);

    GPU::BufferSpec buffer_spec {
        .Label = "SSAO::samples_buffer"sv,
        .Usage = GPU::BufferUsage::Storage,
        .Size = sizeof(Vector3) * 64,
        .Mapped = true,
    };
    samples_buffer = Renderer::Device().CreateBuffer(buffer_spec);

    std::uniform_real_distribution<float> random(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator {};
    generator.seed(1);

    std::array<Vector3, 64> samples;
    u32 i = 0;
    for (auto& sample : samples) {
        sample = Vector3(
            random(generator) * 2.0 - 1.0,
            random(generator) * 2.0 - 1.0,
            random(generator));
        sample.Normalize();

        auto scale = CAST(f32, i++) / 64.0f;
        scale = Math::Lerp(0.1f, 0.5f, scale * scale);
        sample *= scale;
    }

    Mem::Copy(samples_buffer.Slice().MappedRange(), samples.data(), buffer_spec.Size);

    samples_buffer.Unmap();

    std::array<Vector4, 16> noise_values;
    for (auto& noise : noise_values) {
        noise = Vector4(
            random(generator) * 2.0 - 1.0,
            random(generator) * 2.0 - 1.0,
            0.0, 1.0);
    }

    Renderer::Device().WriteTexture(
        noise_texture,
        noise_values.data(),
        16 * sizeof(Vector4),
        Vector2::Zero,
        Vector2 { 4, 4 },
        sizeof(Vector4));

    Options = UniformBuffer<PostProcessing::SSAO>::Create(Renderer::Device(), "SSAO Options"sv);

    UpdateBindGroup(gbuffer);

    std::array bind_group_layouts {
        global_bind_group_layout,
        bind_group_layout,
    };
    GPU::PipelineLayoutSpec pl_spec {
        .BindGroupLayouts = bind_group_layouts
    };
    auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

    GPU::RenderPipelineSpec rp_spec {
        .Label = "SSAO::pipeline"sv,
        .Layout = layout,
        .Vertex = {},
        .Primitive = GPU::PrimitiveState::Default(),
        .DepthStencil = None(),
        .MultiSample = GPU::MultiSampleState::Default(),
        .Fragment = GPU::FragmentStage {
            .Targets = {
                GPU::ColorTargetState {
                    .Format = GPU::TextureFormat::R32Float,
                    .Blend = None(),
                    .WriteMask = GPU::ColorWrite::All,
                },
            } },
    };

    pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
}

void SSAO::resize(Vector2 const& new_size, GBuffer const& gbuffer)
{
    render_target.Release();
    GPU::TextureSpec spec {
        .Label = "SSAO::render_target"sv,
        .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3 { new_size, 1 },
        .Format = GPU::TextureFormat::R32Float,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::All,
    };
    render_target = Renderer::Device().CreateTexture(spec);

    UpdateBindGroup(gbuffer);
}

void SSAO::UpdateBindGroup(GBuffer const& gbuffer)
{
    bind_group.Release();
    std::array bing_group_entries {
        GPU::BindGroupEntry {
            .Binding = 0,
            .Resource = gbuffer.rt_position.View,
        },
        GPU::BindGroupEntry {
            .Binding = 1,
            .Resource = gbuffer.rt_normal.View,
        },
        GPU::BindGroupEntry {
            .Binding = 2,
            .Resource = noise_texture.View,
        },
        GPU::BindGroupEntry {
            .Binding = 3,
            .Resource = sampler,
        },
        GPU::BindGroupEntry {
            .Binding = 4,
            .Resource = noise_sampler,
        },
        GPU::BindGroupEntry {
            .Binding = 5,
            .Resource = GPU::BufferBinding {
                .TargetBuffer = samples_buffer,
                .Offset = 0,
                .Size = samples_buffer.Size() },
        },
        GPU::BindGroupEntry {
            .Binding = 6,
            .Resource = GPU::BufferBinding {
                .TargetBuffer = Options.Buffer(),
                .Offset = 0,
                .Size = Options.Buffer().Size(),
            },
        },
    };
    GPU::BindGroupSpec bg_spec {
        .Label = "SSAO::bing_group"sv,
        .Entries = bing_group_entries
    };
    bind_group = Renderer::Device().CreateBindGroup(bind_group_layout, bg_spec);
}

void SceneRenderer::setup_scene_bind_group()
{
    std::array scene_entries {
        GPU::BindGroupLayoutEntry {
            .Binding = 0,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Texture {
                .SampleType = GPU::TextureSampleType::Float {},
                .ViewDimension = GPU::TextureViewDimension::D2,
                .MultiSampled = false,
            },
            .Count = 1,
        },
        // Environment Map
        GPU::BindGroupLayoutEntry {
            .Binding = 1,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Texture {
                .SampleType = GPU::TextureSampleType::Float {},
                .ViewDimension = GPU::TextureViewDimension::Cube,
                .MultiSampled = false,
            },
            .Count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .Binding = 2,
            .Visibility = GPU::ShaderStage::Fragment,
            .Type = GPU::BindingType::Sampler { .Type = GPU::SamplerBindingType::Filtering },
            .Count = 1,
        },
    };

    GPU::BindGroupLayoutSpec scene_bgl_spec = {
        .Label = "Scene BGL"sv,
        .Entries = scene_entries,
    };

    m_scene_bind_group_layout = Renderer::Device().CreateBindGroupLayout(scene_bgl_spec);

    GPU::TextureView env_view = Renderer::WhiteCubeTexture().View;
    if (m_render_context.EnvironmentMap != nullptr && m_environment_maps.contains(m_render_context.EnvironmentMap->GetHandle())) {
        env_view = m_environment_maps.at(m_render_context.EnvironmentMap->GetHandle()).View;
    }
    std::array scene_bind_group_entries = {
        GPU::BindGroupEntry {
            .Binding = 0,
            .Resource = ssao_blur.render_target().View,
        },
        GPU::BindGroupEntry {
            .Binding = 1,
            .Resource = env_view,
        },
        GPU::BindGroupEntry {
            .Binding = 2,
            .Resource = m_linear_sampler }
    };

    GPU::BindGroupSpec scene_bg_spec {
        .Label = "Scene Bind Group"sv,
        .Entries = scene_bind_group_entries
    };

    m_scene_bind_group = Renderer::Device().CreateBindGroup(m_scene_bind_group_layout, scene_bg_spec);
}

void SceneRenderer::update_scene_bind_group(GPU::Texture const& ssao_texture)
{
    m_scene_bind_group.Release();

    GPU::TextureView env_view = Renderer::WhiteCubeTexture().View;
    if (m_render_context.EnvironmentMap != nullptr && m_environment_maps.contains(m_render_context.EnvironmentMap->GetHandle())) {
        env_view = m_environment_maps.at(m_render_context.EnvironmentMap->GetHandle()).View;
    }
    std::array scene_bind_group_entries = {
        GPU::BindGroupEntry {
            .Binding = 0,
            .Resource = ssao_texture.View,
        },
        GPU::BindGroupEntry {
            .Binding = 1,
            .Resource = env_view,
        },
        GPU::BindGroupEntry {
            .Binding = 2,
            .Resource = m_linear_sampler }
    };

    GPU::BindGroupSpec scene_bg_spec {
        .Label = "Scene Bind Group"sv,
        .Entries = scene_bind_group_entries
    };

    m_scene_bind_group = Renderer::Device().CreateBindGroup(m_scene_bind_group_layout, scene_bg_spec);
}

void SceneRenderer::init()
{
    auto window_size = Application::Self()->GetWindow().Size();

    create_scene_render_target(window_size);

    scene_view_data = UniformBuffer<ViewData>::Create(Renderer::Device(), std::string_view { "View Data" });
    scene_light_data = UniformBuffer<LightData>::Create(Renderer::Device(), std::string_view { "Light Data" });

    ///////////////////////
    /// BIND GROUP CREATION
    ///////////////////////

    setup_shadow_pass_render_target();

    {
        std::array entries {
            GPU::BindGroupLayoutEntry {
                .Binding = 0,
                .Visibility = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Buffer {
                    .Type = GPU::BufferBindingType::Uniform {},
                    .HasDynamicOffset = false,
                    .MinBindingSize = None(),
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 1,
                .Visibility = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Buffer {
                    .Type = GPU::BufferBindingType::Uniform {},
                    .HasDynamicOffset = false,
                    .MinBindingSize = None(),
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 2,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture {
                    .SampleType = GPU::TextureSampleType::Depth {},
                    .ViewDimension = GPU::TextureViewDimension::D2_Array,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
        };

        GPU::BindGroupLayoutSpec spec {
            .Label = "Global BGL"sv,
            .Entries = entries,
        };

        m_global_bind_group_layout = Renderer::Device().CreateBindGroupLayout(spec);

        std::array bind_group_entries {
            GPU::BindGroupEntry {
                .Binding = 0,
                .Resource = GPU::BufferBinding {
                    .TargetBuffer = scene_view_data.Buffer(),
                    .Offset = 0,
                    .Size = UniformBuffer<ViewData>::Size(),
                } },
            GPU::BindGroupEntry { .Binding = 1, .Resource = GPU::BufferBinding {
                                                    .TargetBuffer = scene_light_data.Buffer(),
                                                    .Offset = 0,
                                                    .Size = UniformBuffer<LightData>::Size(),
                                                } },
            GPU::BindGroupEntry { .Binding = 2, .Resource = m_shadow_pass_render_target.View }
        };

        GPU::BindGroupSpec global_bg_spec {
            .Label = "Global Bind Group"sv,
            .Entries = bind_group_entries
        };

        m_global_bind_group = Renderer::Device().CreateBindGroup(m_global_bind_group_layout, global_bg_spec);
    }

    {
        std::array entries {
            GPU::BindGroupLayoutEntry {
                .Binding = 0,
                .Visibility = GPU::ShaderStage::Vertex,
                .Type = GPU::BindingType::Buffer {
                    .Type = GPU::BufferBindingType::Storage {
                        .ReadOnly = true,
                    },
                    .HasDynamicOffset = false,
                    .MinBindingSize = None(),
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 1,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Buffer {
                    .Type = GPU::BufferBindingType::Uniform {},
                    .HasDynamicOffset = false,
                    .MinBindingSize = None(),
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 2,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture {
                    .SampleType = GPU::TextureSampleType::Float {},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 3,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture {
                    .SampleType = GPU::TextureSampleType::Float {},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 4,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture {
                    .SampleType = GPU::TextureSampleType::Float {},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 5,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture {
                    .SampleType = GPU::TextureSampleType::Float {},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 6,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture {
                    .SampleType = GPU::TextureSampleType::Float {},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 7,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Sampler {
                    .Type = GPU::SamplerBindingType::Filtering,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .Binding = 8,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Sampler {
                    .Type = GPU::SamplerBindingType::Comparison,
                },
                .Count = 1,
            },

        };

        GPU::BindGroupLayoutSpec spec {
            .Label = "Object BGL"sv,
            .Entries = entries,
        };

        m_object_bind_group_layout = Renderer::Device().CreateBindGroupLayout(spec);
    }

    ////////////////////////
    /// RENDER PASS CREATION
    ////////////////////////

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/simple.wgsl").Unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .Label = "Simple WGSL Shader"sv,
            .Type = GPU::WGSLShader {
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts {
            m_global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec {
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        GPU::RenderPipelineSpec rp_spec {
            .Label = "Simple RP"sv,
            .Layout = layout,
            .Vertex = {},
            .Primitive = GPU::PrimitiveState::Default(),
            .DepthStencil = None(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = GPU::FragmentStage {
                .Targets = {
                    GPU::ColorTargetState {
                        .Format = GPU::TextureFormat::RGBA8Unorm,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    } } },
        };

        m_simple_pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/Editor/Grid.wgsl").Unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .Label = "Grid Shader"sv,
            .Type = GPU::WGSLShader {
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts {
            m_global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec {
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        GPU::RenderPipelineSpec rp_spec {
            .Label = "Grid RP"sv,
            .Layout = layout,
            .Vertex = {},
            .Primitive = GPU::PrimitiveState::Default(),
            .DepthStencil = GPU::DepthStencilState::Default(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = GPU::FragmentStage {
                .Targets = {
                    GPU::ColorTargetState {
                        .Format = TonemappingPipeline::Format,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    } } },
        };

        m_grid_pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/Sky.wgsl").Unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .Label = "Sky Shader:VS"sv,
            .Type = GPU::WGSLShader {
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts {
            m_global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec {
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        auto primitive = GPU::PrimitiveState::Default();
        primitive.Topology = GPU::PrimitiveTopology::TriangleStrip;

        auto depth = GPU::DepthStencilState::Default();
        depth.DepthWriteEnabled = false;
        depth.DepthCompare = GPU::CompareFunction::Always;
        GPU::RenderPipelineSpec rp_spec {
            .Label = "Sky RP"sv,
            .Layout = layout,
            .Vertex = {},
            .Primitive = primitive,
            .DepthStencil = depth,
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = GPU::FragmentStage {
                .Targets = {
                    GPU::ColorTargetState {
                        .Format = TonemappingPipeline::Format,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    },
                },
            },
        };

        m_sky_pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
    }

    GPU::BufferSpec ibs {
        .Label = "PBR Instance Buffer"sv,
        .Usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        .Size = sizeof(Mat4) * 2'000,
        .Mapped = false,
    };
    m_pbr_instance_buffer = Renderer::Device().CreateBuffer(ibs);

    m_pbr_instance_staging_buffer.reserve(sizeof(Mat4) * 2'000);

    GPU::SamplerSpec bilinear_sampler_spec {
        .label = "Bilinear Sampler"sv,
        .AddressModeU = GPU::AddressMode::Repeat,
        .AddressModeV = GPU::AddressMode::Repeat,
        .AddressModeW = GPU::AddressMode::Repeat,
        .MagFilter = GPU::FilterMode::Linear,
        .MinFilter = GPU::FilterMode::Linear,
        .MipMapFilter = GPU::FilterMode::Linear,
        .LodMinClamp = 0.f,
        .LodMaxClamp = 32.f,
        .AnisotropyClamp = 16
    };

    m_linear_sampler = Renderer::Device().CreateSampler(bilinear_sampler_spec);
    bilinear_sampler_spec.MagFilter = GPU::FilterMode::Linear;
    bilinear_sampler_spec.MinFilter = GPU::FilterMode::Linear;
    bilinear_sampler_spec.AnisotropyClamp = 1_u16;
    bilinear_sampler_spec.CompareFunc = GPU::CompareFunction::LessEqual;

    m_shadow_sampler = Renderer::Device().CreateSampler(bilinear_sampler_spec);

    m_hdr_pipeline.init(window_size, m_scene_render_target.Spec.Format);

    gbuffer.init(window_size, m_global_bind_group_layout);
    ssao.init(window_size, gbuffer, m_global_bind_group_layout);
    ssao_blur.init(window_size);

    setup_scene_bind_group();

    // Creating the pbr pipeline after the scene bind group, which must be done after the ssao blur pipeline. oof incarnate.
    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/PBR.wgsl").Unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .Label = "PBR Shader"sv,
            .Type = GPU::WGSLShader {
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts {
            m_global_bind_group_layout,
            m_scene_bind_group_layout,
            m_object_bind_group_layout,
        };
        GPU::PipelineLayoutSpec pl_spec {
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        std::array attributes {
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 0,
            },
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 1,
            },
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float4,
                .ShaderLocation = 2,
            },
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float2,
                .ShaderLocation = 3,
            },
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 4,
            },
        };
        auto attribute_layout = GPU::VertexBufferLayout::Create(attributes);

        GPU::RenderPipelineSpec rp_spec {
            .Label = "PBR Render Pass"sv,
            .Layout = layout,
            .Vertex = {
                .AttributeLayouts = { attribute_layout } },
            .Primitive = {
                .Topology = GPU::PrimitiveTopology::TriangleList,
                .StripIndexFormat = None(),
                .FrontFace = GPU::FrontFace::Ccw,
                .Cull = GPU::Face::None,
            },
            .DepthStencil = GPU::DepthStencilState::Default(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = GPU::FragmentStage {
                .Targets = {
                    GPU::ColorTargetState {
                        .Format = TonemappingPipeline::Format,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    },
                },
            },
        };

        m_pbr_pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
    }

    m_cube_skybox.init({ m_global_bind_group_layout, m_scene_bind_group_layout });

    Debug::Initialize(Renderer::Device(), m_global_bind_group_layout, m_hdr_pipeline.Format);

    setup_shadow_pass();

    setup_queries();
}

void SceneRenderer::resize(Vector2 const& new_size)
{
    ZoneScoped;
    m_render_area = new_size;

    m_scene_render_target.Release();
    m_scene_render_depth_target.Release();
    create_scene_render_target(new_size);
    m_hdr_pipeline.resize(new_size);

    gbuffer.resize(new_size);
    ssao.resize(new_size, gbuffer);
    ssao_blur.resize(new_size, ssao.render_target);
    update_scene_bind_group(ssao_blur.render_target());
}

f32 GetSplitDepth(s32 current_split, s32 max_splits, f32 near, f32 far, f32 l = 1.0f)
{
    auto split_ratio = CAST(f32, current_split) / CAST(f32, max_splits);
    auto log = near * Math::Pow(far / near, split_ratio);
    auto uniform = near + (far - near) * split_ratio;
    auto d = l * (log - uniform) + uniform;
    return (d - near) / (far - near);
}

void SceneRenderer::render(GPU::CommandEncoder& encoder, RenderPacket const& packet, bool game_view)
{
    ZoneScoped;

    if (Renderer::HasPipelineStatistics()) {
        if (m_timings_read_buffer.GetMapState() == GPU::MapState::Unmapped) {
            m_timings_read_buffer.Slice().MapAsync(GPU::MapMode::Read, [this] {
                auto data = CAST(u64*, m_timings_read_buffer.Slice().MappedRange());
                timings.gbuffer = CAST(f64, data[3] - data[2]) * 1e-6;
                timings.ssao = CAST(f64, data[5] - data[4]) * 1e-6;
                timings.ssao_blur = CAST(f64, data[7] - data[6]) * 1e-6;
                timings.pbr = CAST(f64, data[9] - data[8]) * 1e-6;

                m_timings_read_buffer.Unmap();
            });
        }

        if (m_statistics_read_buffer.GetMapState() == GPU::MapState::Unmapped) {
            m_statistics_read_buffer.Slice().MapAsync(GPU::MapMode::Read, [this] {
                auto data = CAST(u64*, m_statistics_read_buffer.Slice().MappedRange());
                // 1st VertexShaderInvocations
                // 2nd ClipperInvocations
                // 3d FragmentShaderInvocations

                u32 i = 0;
                pipeline_statistics.gbuffer.vertex_shader_invocations = data[i++];
                pipeline_statistics.gbuffer.clipper_invocations = data[i++];
                pipeline_statistics.gbuffer.fragment_shader_invocations = data[i++];

                pipeline_statistics.ssao.vertex_shader_invocations = data[i++];
                pipeline_statistics.ssao.clipper_invocations = data[i++];
                pipeline_statistics.ssao.fragment_shader_invocations = data[i++];

                pipeline_statistics.pbr.vertex_shader_invocations = data[i++];
                pipeline_statistics.pbr.clipper_invocations = data[i++];
                pipeline_statistics.pbr.fragment_shader_invocations = data[i++];

                m_statistics_read_buffer.Unmap();
            });
        }
    }

    m_render_context.Reset();

    {
        ZoneScopedN("Render Object Collection");
        m_render_context.RenderFlags = RenderState::LightCollection;
        if (packet.scene) {
            packet.scene->ForEachEntity([&](Entity* entity) {
                entity->OnDraw(m_render_context);
            });
        }
    }

    scene_view_data.Data.perspective = packet.camera.perspective;
    scene_view_data.Data.view = packet.camera.view;
    scene_view_data.Data.view_rotation_only = packet.camera.rotation;
    scene_view_data.Data.position = packet.camera.position;
    scene_view_data.Data.screen_size = m_render_area;
    scene_view_data.flush();

    if (!m_render_context.DirectionalLights.empty()) {
        scene_light_data.Data.directional_light = m_render_context.DirectionalLights[0].ShaderData;
        scene_light_data.Data.shadow_split_distances = Vector4 { 0.0f };
    }
    scene_light_data.flush();

    depth_pass(encoder, packet);
    if (m_render_context.EnvironmentMap != nullptr) {
        if (auto handle = m_render_context.EnvironmentMap->GetHandle(); !m_environment_maps.contains(handle)) {
            IBLIrradiance ibl_irradiance {};
            ibl_irradiance.init();
            auto texture = ibl_irradiance.generate(m_render_context.EnvironmentMap->GetTexture());
            m_environment_maps[handle] = texture;
            // Somehow this causes a User-mode data execution prevention (DEP) violation
            // m_environment_maps[handle] = gen.generate(m_render_context.environment_map->image());
        }
    }
    pbr_pass(encoder, packet, game_view);

    m_hdr_pipeline.process(encoder, m_scene_render_target.View, m_render_context);

    for (auto& group : m_object_groups_to_release) {
        group.Release();
    }
    m_object_groups_to_release.clear();

    Debug::Reset();
}

void SceneRenderer::setup_shadow_pass_render_target()
{
    GPU::TextureSpec spec {
        .Label = "DepthPass::RenderTarget"sv,
        .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = { SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION, MAX_SHADOW_CASCADES },
        .Format = GPU::TextureFormat::Depth24Plus,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::DepthOnly,
    };
    m_shadow_pass_render_target = Renderer::Device().CreateTexture(spec);
    m_shadow_pass_render_target.InitializeView(MAX_SHADOW_CASCADES);

    for (u32 i = 0; i < MAX_SHADOW_CASCADES; ++i) {
        m_shadow_pass_render_target_views[i] = m_shadow_pass_render_target.CreateView({
            .Label = "Shadow Pass Render Target"sv,
            .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .Dimension = GPU::TextureViewDimension::D2,
            .Format = GPU::TextureFormat::Depth24Plus,
            .BaseMipLevel = 0,
            .MipLevelCount = 1,
            .BaseArrayLayer = i,
            .ArrayLayerCount = 1,
            .Aspect = GPU::TextureAspect::DepthOnly,
        });
    }
}

void SceneRenderer::setup_shadow_pass()
{
    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/DepthPass.wgsl").Unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .Label = "PBR Shader"sv,
            .Type = GPU::WGSLShader {
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            //.FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bgl_entries {
            GPU::BindGroupLayoutEntry {
                .Binding = 0,
                .Visibility = GPU::ShaderStage::Vertex,
                .Type = GPU::BindingType::Buffer {
                    .Type = GPU::BufferBindingType::Storage {
                        .ReadOnly = true,
                    },
                },
                .Count = 1,
            },
        };
        GPU::BindGroupLayoutSpec bgl_spec {
            .Entries = bgl_entries,
        };

        m_object_depth_bgl = Renderer::Device().CreateBindGroupLayout(bgl_spec);

        std::array bind_group_layouts {
            m_object_depth_bgl,
        };
        GPU::PipelineLayoutSpec pl_spec {
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        std::array attributes {
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 0,
            },
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 1,
            },
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float4,
                .ShaderLocation = 2,
            },
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float2,
                .ShaderLocation = 3,
            },
            GPU::VertexAttribute {
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 4,
            },
        };
        auto attribute_layout = GPU::VertexBufferLayout::Create(attributes);

        GPU::RenderPipelineSpec rp_spec {
            .Label = "DepthPass::Pipeline"sv,
            .Layout = layout,
            .Vertex = {
                .AttributeLayouts = { attribute_layout } },
            .Primitive = {
                .Topology = GPU::PrimitiveTopology::TriangleList,
                .StripIndexFormat = None(),
                .FrontFace = GPU::FrontFace::Ccw,
                .Cull = GPU::Face::None,
            },
            .DepthStencil = GPU::DepthStencilState::Default(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = None(),
        };

        m_depth_pipeline = Renderer::Device().CreateRenderPipeline(shader, shader, rp_spec);
    }

    GPU::BufferSpec ibs {
        .Label = "Depth Instance Buffer"sv,
        .Usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        .Size = sizeof(Mat4) * 2'000,
        .Mapped = false,
    };
    m_depth_instance_buffer = Renderer::Device().CreateBuffer(ibs);

    m_depth_instance_staging_buffer.reserve(sizeof(Mat4) * 2'000);
}

void SceneRenderer::depth_pass(GPU::CommandEncoder& encoder, RenderPacket const& packet)
{
    {
        ZoneScopedN("Depth Pass");

        for (auto& light : m_render_context.DirectionalLights) {

            std::array<f32, MAX_SHADOW_CASCADES> shadow_splits {};
            for (auto i = 0; i < MAX_SHADOW_CASCADES; i++) {
                // SceneLightData.Data.ShadowSplitDistances[i] = GetSplitDepth(i + 1, ShadowCascades, packet.Camera.Near, packet.Camera.Far, light.Split);
                shadow_splits[i] = GetSplitDepth(i + 1, MAX_SHADOW_CASCADES, packet.camera.near, packet.camera.far, light.Split);
            }

            GPU::Buffer instance_buffer;
            if (!m_instance_buffer_pool.empty()) {
                instance_buffer = m_instance_buffer_pool.back();
                m_instance_buffer_pool.pop_back();
            } else {
                GPU::BufferSpec spec {
                    .Label = "Depth Instance Buffer"sv,
                    .Usage = GPU::BufferUsage::CopySrc | GPU::BufferUsage::MapWrite,
                    .Size = sizeof(Mat4) * 2 * 2'000,
                    .Mapped = true,
                };
                instance_buffer = Renderer::Device().CreateBuffer(spec);
            }

            f32 last_split { 0 };
            usz buffer_offset = 0;
            for (auto i = 0; i < MAX_SHADOW_CASCADES; i++) {
                ZoneScopedN("Shadow Cascade");

                Vector3 frustum_corners[8] = {
                    Vector3(-1.0f, 1.0f, 0.0f),
                    Vector3(1.0f, 1.0f, 0.0f),
                    Vector3(1.0f, -1.0f, 0.0f),
                    Vector3(-1.0f, -1.0f, 0.0f),
                    Vector3(-1.0f, 1.0f, 1.0f),
                    Vector3(1.0f, 1.0f, 1.0f),
                    Vector3(1.0f, -1.0f, 1.0f),
                    Vector3(-1.0f, -1.0f, 1.0f),
                };

                // Project frustum corners into world space
                glm::mat4 inv_cam = glm::inverse(packet.camera.perspective * packet.camera.view);
                for (auto& frustum_corner : frustum_corners) {
                    Vector4 inv_corner = inv_cam * Vector4(frustum_corner, 1.0f);
                    frustum_corner = inv_corner / inv_corner.w;
                }

                auto split = shadow_splits[i];
                for (u32 j = 0; j < 4; j++) {
                    Vector3 dist = frustum_corners[j + 4] - frustum_corners[j];
                    frustum_corners[j + 4] = frustum_corners[j] + dist * split;
                    frustum_corners[j] = frustum_corners[j] + dist * last_split;
                }

                Vector3 center {};
                for (auto frustum_corner : frustum_corners) {
                    center += frustum_corner;
                }
                center /= 8.0f;

                float radius = 0.0f;
                for (auto frustum_corner : frustum_corners) {
                    f32 distance = (frustum_corner - center).Length();
                    radius = Math::Max(radius, distance);
                }
                radius = std::ceil(radius * 16.f) / 16.f;

                auto texel_size = CAST(f32, SHADOWMAP_RESOLUTION) / (radius * 2.0f);
                Mat4 scalar(texel_size);

                Vector3 max_extents { radius, radius, radius };
                Vector3 min_extents = -max_extents;

                glm::mat4 view = glm::lookAt(glm::vec3(center - Vector3(light.ShaderData.Direction) * min_extents.z), glm::vec3(center), glm::vec3(Vector3::Up));
                view = scalar * view;
                center = glm::mat3(view) * center;
                center.x = Math::Floor(center.x);
                center.y = Math::Floor(center.y);
                center = inverse(glm::mat3(view)) * center;
                view = glm::lookAt(glm::vec3(center - Vector3(light.ShaderData.Direction) * min_extents.z), glm::vec3(center), glm::vec3(Vector3::Up));

                glm::mat4 proj = glm::ortho(min_extents.x, max_extents.x, min_extents.y, max_extents.y, -20.0f, max_extents.z - min_extents.z);

                light.ShaderData.LightSpaceMatrix[i] = proj * view;

                scene_light_data.Data.shadow_split_distances[i] = packet.camera.near + split * (packet.camera.far - packet.camera.near) * -1.0f;
                scene_light_data.Data.directional_light.LightSpaceMatrix[i] = light.ShaderData.LightSpaceMatrix[i];

                for (auto const& [material, mesh_map] : m_render_context.MeshRenderLists) {
                    (void)material;
                    for (auto const& [buffer, list] : mesh_map) {
                        (void)buffer;
                        auto data = TRANSMUTE(DepthInstanceData*, instance_buffer.Slice().MappedRange()) + buffer_offset;
                        int j = 0;
                        for (auto index : list) {
                            auto& obj = m_render_context.RenderObjects[index];
                            data[j].model = obj.WorldMatrix;
                            data[j++].light_space = light.ShaderData.LightSpaceMatrix[i];
                        }

                        buffer_offset += list.size();
                    }
                }
                last_split = split;
            }

            instance_buffer.Unmap();
            auto copy_encoder = Renderer::Device().CreateCommandEncoder();

            copy_encoder.CopyBufferToBuffer(instance_buffer, 0, m_depth_instance_buffer, 0, buffer_offset * sizeof(DepthInstanceData));
            Renderer::Device().SubmitCommandBuffer(copy_encoder.Finish());
            copy_encoder.Release();

            instance_buffer.Slice().MapAsync(GPU::MapMode::Write, [instance_buffer, this] {
                auto& copy = m_instance_buffer_pool.emplace_back(instance_buffer);
                copy.ForceMapState(GPU::MapState::Mapped);
            });

            buffer_offset = 0;
            for (auto i = 0; i < MAX_SHADOW_CASCADES; i++) {
                GPU::RenderPassSpec rp_spec {
                    .Label = "DepthPass::RenderPass"sv,
                    .DepthStencilAttachment = GPU::RenderPassColorAttachment {
                        .View = m_shadow_pass_render_target_views[i],
                        .LoadOp = GPU::LoadOp::Clear,
                        .StoreOp = GPU::StoreOp::Store,
                        .DepthClear = 1.0f,
                    },
                };
                auto rp = encoder.BeginRendering(rp_spec);
                rp.SetViewport({}, { SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION });
                rp.SetPipeline(m_depth_pipeline);

                std::array bind_group_entries {
                    GPU::BindGroupEntry {
                        .Binding = 0,
                        .Resource = GPU::BufferBinding {
                            .TargetBuffer = m_depth_instance_buffer,
                            .Offset = 0,
                            .Size = m_depth_instance_buffer.Size(),
                        } },
                };

                GPU::BindGroupSpec bg_spec {
                    .Label = "Object Depth Bind Group"sv,
                    .Entries = bind_group_entries
                };

                auto object_group = Renderer::Device().CreateBindGroup(m_object_depth_bgl, bg_spec);
                m_object_groups_to_release.push_back(object_group);
                rp.SetBindGroup(object_group, 0);
                for (auto const& [material, mesh_map] : m_render_context.MeshRenderLists) {

                    for (auto const& [buffer, list] : mesh_map) {
                        if (list.empty())
                            continue;
                        (void)buffer;

                        auto& hack = m_render_context.RenderObjects[list[0]];
                        rp.SetVertexBuffer(0, hack.VertexBuffer);
                        rp.SetIndexBuffer(hack.IndexBuffer);

                        rp.DrawIndex({ 0, hack.IndexCount }, { CAST(u32, buffer_offset), CAST(u32, list.size()) });
                        buffer_offset += list.size();
                    }
                }
                rp.End();
                rp.Release();
            }
            scene_light_data.flush();
        }
    }
}

void SceneRenderer::pbr_pass(GPU::CommandEncoder const& encoder, RenderPacket const& packet, bool game_view)
{
    (void)packet;
    GPU::Buffer instance_buffer;
    if (!m_instance_buffer_pool.empty()) {
        instance_buffer = m_instance_buffer_pool.back();
        m_instance_buffer_pool.pop_back();
    } else {
        GPU::BufferSpec spec {
            .Label = "PBR Instance Buffer"sv,
            .Usage = GPU::BufferUsage::CopySrc | GPU::BufferUsage::MapWrite,
            .Size = sizeof(Mat4) * 2'000,
            .Mapped = true,
        };
        instance_buffer = Renderer::Device().CreateBuffer(spec);
    }

    usz buffer_count_offset = 0;

    for (auto const& [material, mesh_map] : m_render_context.MeshRenderLists) {
        for (auto const& [buffer, list] : mesh_map) {
            VERIFY(instance_buffer.GetMapState() == GPU::MapState::Mapped, "{}", magic_enum::enum_name(instance_buffer.GetMapState()));
            auto data = TRANSMUTE(InstanceData*, instance_buffer.Slice().MappedRange()) + buffer_count_offset;
            int j = 0;
            for (auto index : list) {
                auto& obj = m_render_context.RenderObjects[index];
                data[j++].model = obj.WorldMatrix;
            }

            buffer_count_offset += list.size();
        }
    }
    instance_buffer.Unmap();

    auto copy_encoder = Renderer::Device().CreateCommandEncoder();

    copy_encoder.CopyBufferToBuffer(instance_buffer, 0, m_pbr_instance_buffer, 0, buffer_count_offset * sizeof(InstanceData));
    Renderer::Device().SubmitCommandBuffer(copy_encoder.Finish());
    copy_encoder.Release();

    instance_buffer.Slice().MapAsync(GPU::MapMode::Write, [instance_buffer, this] {
        auto& copy = m_instance_buffer_pool.emplace_back(instance_buffer);
        copy.ForceMapState(GPU::MapState::Mapped);
    });

    {
        ZoneScopedN("G-Buffer");
        std::array color_attachments {
            GPU::RenderPassColorAttachment {
                .View = gbuffer.rt_position.View,
                .LoadOp = GPU::LoadOp::Clear,
                .StoreOp = GPU::StoreOp::Store,
                .ClearColor = Color::Black,
            },
            GPU::RenderPassColorAttachment {
                .View = gbuffer.rt_normal.View,
                .LoadOp = GPU::LoadOp::Clear,
                .StoreOp = GPU::StoreOp::Store,
                .ClearColor = Color::Black,
            },
            GPU::RenderPassColorAttachment {
                .View = gbuffer.rt_albedo.View,
                .LoadOp = GPU::LoadOp::Clear,
                .StoreOp = GPU::StoreOp::Store,
                .ClearColor = Color::Black,
            },
        };

        GPU::RenderPassTimestampWrites timestamp_writes {
            .Set = m_timings_set,
            .BeginningOfPassWriteIndex = 2,
            .EndOfPassWriteIndex = 3
        };
        GPU::RenderPassSpec gpass_rp_spec {
            .Label = "GBuffer::render_pass"sv,
            .ColorAttachments = color_attachments,
            .DepthStencilAttachment = GPU::RenderPassColorAttachment {
                .View = m_scene_render_depth_target.View,
                .LoadOp = GPU::LoadOp::Clear,
                .StoreOp = GPU::StoreOp::Store,
                .DepthClear = 1.0f,
            },
            .TimestampWrites = timestamp_writes,
        };
        auto gpass = encoder.BeginRendering(gpass_rp_spec);
        gpass.SetPipeline(gbuffer.pipeline);
        gpass.SetBindGroup(m_global_bind_group, 0);
        begin_pipeline_statistics_query(gpass, m_statistics_query_set, 0);

        std::array bind_group_entries {
            GPU::BindGroupEntry {
                .Binding = 0,
                .Resource = GPU::BufferBinding {
                    .TargetBuffer = m_pbr_instance_buffer,
                    .Offset = 0,
                    .Size = m_pbr_instance_buffer.Size(),
                } },
        };

        GPU::BindGroupSpec bg_spec {
            .Label = "Object Bind Group"sv,
            .Entries = bind_group_entries
        };

        auto object_group = Renderer::Device().CreateBindGroup(gbuffer.bind_group_layout, bg_spec);
        m_object_groups_to_release.push_back(object_group);
        gpass.SetBindGroup(object_group, 1);
        buffer_count_offset = 0;
        for (auto const& [material, mesh_map] : m_render_context.MeshRenderLists) {

            for (auto& [buffer, list] : mesh_map) {
                if (list.empty())
                    continue;
                (void)buffer;
                auto& hack = m_render_context.RenderObjects[list[0]];
                gpass.SetVertexBuffer(0, hack.VertexBuffer);
                gpass.SetIndexBuffer(hack.IndexBuffer);

                gpass.DrawIndex({ 0, hack.IndexCount }, { CAST(u32, buffer_count_offset), CAST(u32, list.size()) });
                buffer_count_offset += list.size();
            }
        }

        end_pipeline_statistics_query(gpass);
        gpass.End();
        gpass.Release();
    }

    {
        ZoneScopedN("SSAO");
        std::array color_attachments {
            GPU::RenderPassColorAttachment {
                .View = ssao.render_target.View,
                .LoadOp = GPU::LoadOp::Clear,
                .StoreOp = GPU::StoreOp::Store,
                .ClearColor = Color::White,
            },
        };

        GPU::RenderPassTimestampWrites timestamp_writes {
            .Set = m_timings_set,
            .BeginningOfPassWriteIndex = 4,
            .EndOfPassWriteIndex = 5
        };

        GPU::RenderPassSpec rp_spec {
            .Label = "SSAO::render_pass"sv,
            .ColorAttachments = color_attachments,
            .TimestampWrites = timestamp_writes
        };
        auto pass = encoder.BeginRendering(rp_spec);
        begin_pipeline_statistics_query(pass, m_statistics_query_set, 1);

        if (m_render_context.PostProcessingSettings.UseSSAO) {
            ssao.Options.Data = m_render_context.PostProcessingSettings.SSAOData;
            ssao.Options.flush();

            pass.SetPipeline(ssao.pipeline);
            pass.SetBindGroup(m_global_bind_group, 0);
            pass.SetBindGroup(ssao.bind_group, 1);
            pass.Draw({ 0, 3 }, { 0, 1 });
        }

        end_pipeline_statistics_query(pass);
        pass.End();
        pass.Release();
    }

    ssao_blur.draw(encoder, m_timings_set, 6, 7);

    {
        ZoneScopedN("PBR Pass");
        std::array color_attachments {
            GPU::RenderPassColorAttachment {
                .View = m_hdr_pipeline.view(),
                .LoadOp = GPU::LoadOp::Clear,
                .StoreOp = GPU::StoreOp::Store,
                .ClearColor = Color::Black,
            },
        };

        GPU::RenderPassSpec scene_rp_spec {
            .Label = "Scene Render Pass"sv,
            .ColorAttachments = color_attachments,
            .DepthStencilAttachment = GPU::RenderPassColorAttachment {
                .View = m_scene_render_depth_target.View,
                .LoadOp = GPU::LoadOp::Clear,
                .StoreOp = GPU::StoreOp::Store,
                .DepthClear = 1.0f,
            },
            .TimestampWrites = GPU::RenderPassTimestampWrites { .Set = m_timings_set, .BeginningOfPassWriteIndex = 8, .EndOfPassWriteIndex = 9 }
        };
        auto scene_rp = encoder.BeginRendering(scene_rp_spec);
        begin_pipeline_statistics_query(scene_rp, m_statistics_query_set, 2);

        scene_rp.SetViewport(Vector2::Zero, { m_render_area.x, m_render_area.y });

        scene_rp.SetBindGroup(m_global_bind_group, 0);
        scene_rp.SetBindGroup(m_scene_bind_group, 1);
        {
            scene_rp.SetPipeline(m_sky_pipeline);
            scene_rp.Draw({ 0, 4 }, { 0, 1 });
        }

        m_cube_skybox.execute(scene_rp);

        scene_rp.SetPipeline(m_pbr_pipeline);

        // For each material
        // - Create a bind group
        // - Bind it
        // - For each mesh list
        // -- Draw it

        buffer_count_offset = 0;
        for (auto const& [material, mesh_map] : m_render_context.MeshRenderLists) {

            material->UpdateBuffer();

            auto albedo = material->albedo_map.Get();
            if (!albedo) {
                albedo = Renderer::WhiteTexture().Get();
            }

            auto normal = material->normal_map.Get();
            if (!normal) {
                normal = Renderer::DefaultNormalMap().Get();
            }

            auto ao = material->ambient_occlusion_map.Get();
            if (!ao) {
                ao = Renderer::WhiteTexture().Get();
            }

            auto metallic_roughness = material->metallic_roughness_map.Get();
            if (!metallic_roughness) {
                metallic_roughness = Renderer::WhiteTexture().Get();
            }

            auto emissive = material->emissive_map.Get();
            if (!emissive) {
                emissive = Renderer::BlackTexture().Get();
            }

            std::array bind_group_entries {
                GPU::BindGroupEntry {
                    .Binding = 0,
                    .Resource = GPU::BufferBinding {
                        .TargetBuffer = m_pbr_instance_buffer,
                        .Offset = 0,
                        .Size = m_pbr_instance_buffer.Size(),
                    },
                },
                GPU::BindGroupEntry {
                    .Binding = 1,
                    .Resource = GPU::BufferBinding {
                        .TargetBuffer = material->material_uniform_buffer.Buffer(),
                        .Offset = 0,
                        .Size = material->material_uniform_buffer.Buffer().Size(),
                    },
                },
                GPU::BindGroupEntry {
                    .Binding = 2,
                    .Resource = albedo->GetTexture().View,
                },
                GPU::BindGroupEntry {
                    .Binding = 3,
                    .Resource = normal->GetTexture().View,
                },
                GPU::BindGroupEntry {
                    .Binding = 4,
                    .Resource = metallic_roughness->GetTexture().View,
                },
                GPU::BindGroupEntry {
                    .Binding = 5,
                    .Resource = ao->GetTexture().View,
                },
                GPU::BindGroupEntry {
                    .Binding = 6,
                    .Resource = emissive->GetTexture().View,
                },
                GPU::BindGroupEntry {
                    .Binding = 7,
                    .Resource = m_linear_sampler,
                },
                GPU::BindGroupEntry {
                    .Binding = 8,
                    .Resource = m_shadow_sampler,
                },
            };

            GPU::BindGroupSpec global_bg_spec {
                .Label = "Object Bind Group"sv,
                .Entries = bind_group_entries
            };

            auto object_group = Renderer::Device().CreateBindGroup(m_object_bind_group_layout, global_bg_spec);
            m_object_groups_to_release.push_back(object_group);
            scene_rp.SetBindGroup(object_group, 2);

            for (auto const& [buffer, list] : mesh_map) {
                if (list.empty())
                    continue;
                (void)buffer;
                auto& hack = m_render_context.RenderObjects[list[0]];

                scene_rp.SetVertexBuffer(0, hack.VertexBuffer);
                scene_rp.SetIndexBuffer(hack.IndexBuffer);

                scene_rp.DrawIndex({ 0, hack.IndexCount }, { CAST(u32, buffer_count_offset), CAST(u32, list.size()) });
                buffer_count_offset += list.size();
            }
        }
        // Draw editor specific stuff only if we are not rendering a game view.
        if (!game_view) {
            Debug::Render(scene_rp);

            scene_rp.SetPipeline(m_grid_pipeline);
            scene_rp.Draw({ 0, 6 }, { 0, 1 });
        }

        end_pipeline_statistics_query(scene_rp);
        scene_rp.End();
        scene_rp.Release();
    }

    if (Renderer::HasPipelineStatistics()) {
        encoder.ResolveQuerySet(m_timings_set, { 0, 10 }, m_timings_resolve_buffer, 0);
        encoder.ResolveQuerySet(m_statistics_query_set, { 0, 3 }, m_statistics_resolve_buffer, 0);

        if (m_timings_read_buffer.GetMapState() == GPU::MapState::Unmapped) {
            encoder.CopyBufferToBuffer(m_timings_resolve_buffer, 0, m_timings_read_buffer, 0, m_timings_read_buffer.Size());
        }
        if (m_statistics_read_buffer.GetMapState() == GPU::MapState::Unmapped) {
            encoder.CopyBufferToBuffer(m_statistics_resolve_buffer, 0, m_statistics_read_buffer, 0, m_statistics_read_buffer.Size());
        }
    }
}

void SceneRenderer::setup_queries()
{

    {
        GPU::QuerySetSpec query_set_spec {
            .Label = String("Timings Set"),
            .Type = GPU::QueryType::Timestamp {},
            .Count = 5 * 2, // 5, one for each pass, times 2 for start and end
        };

        m_timings_set = Renderer::Device().CreateQuerySet(query_set_spec);

        GPU::BufferSpec resolve_spec {
            .Label = "Timings Resolve Buffer"sv,
            .Usage = GPU::BufferUsage::QueryResolve | GPU::BufferUsage::CopySrc,
            .Size = query_set_spec.Count * 8, // Each element in a querySet takes 8 bytes: https://webgpufundamentals.org/webgpu/lessons/webgpu-timing.html
            .Mapped = false
        };

        m_timings_resolve_buffer = Renderer::Device().CreateBuffer(resolve_spec);

        GPU::BufferSpec read_spec {
            .Label = "Timings Read Buffer"sv,
            .Usage = GPU::BufferUsage::MapRead | GPU::BufferUsage::CopyDst,
            .Size = resolve_spec.Size,
            .Mapped = false
        };

        m_timings_read_buffer = Renderer::Device().CreateBuffer(read_spec);
    }

    if (Renderer::HasPipelineStatistics()) {
        using enum GPU::PipelineStatisticName;
        GPU::QuerySetSpec query_set_spec {
            .Label = String("Pipeline Statistics Set"),
            .Type = FragmentShaderInvocations | VertexShaderInvocations | ClipperInvocations,
            .Count = 4 * 3, // 5, one for each pass, times 3, for each of the types above
        };

        m_statistics_query_set = Renderer::Device().CreateQuerySet(query_set_spec);

        GPU::BufferSpec resolve_spec {
            .Label = "Pipeline Statistics Resolve Buffer"sv,
            .Usage = GPU::BufferUsage::QueryResolve | GPU::BufferUsage::CopySrc,
            .Size = query_set_spec.Count * 8, // Each element in a querySet takes 8 bytes: https://webgpufundamentals.org/webgpu/lessons/webgpu-timing.html
            .Mapped = false
        };

        m_statistics_resolve_buffer = Renderer::Device().CreateBuffer(resolve_spec);

        GPU::BufferSpec read_spec {
            .Label = "Pipeline Statistics Read Buffer"sv,
            .Usage = GPU::BufferUsage::MapRead | GPU::BufferUsage::CopyDst,
            .Size = resolve_spec.Size,
            .Mapped = false
        };

        m_statistics_read_buffer = Renderer::Device().CreateBuffer(read_spec);
    }
}

void SceneRenderer::create_scene_render_target(Vector2 const& size)
{
    GPU::TextureSpec spec {
        .Label = "Main Render Target"sv,
        .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3 { size, 1 },
        .Format = GPU::TextureFormat::RGBA8UnormSrgb,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::All,
    };
    m_scene_render_target = Renderer::Device().CreateTexture(spec);

    GPU::TextureSpec depth_spec {
        .Label = "Main Render Depth Target"sv,
        .Usage = GPU::TextureUsage::RenderAttachment,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3 { size, 1 },
        .Format = GPU::TextureFormat::Depth24Plus,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::DepthOnly,
    };
    m_scene_render_depth_target = Renderer::Device().CreateTexture(depth_spec);
}
