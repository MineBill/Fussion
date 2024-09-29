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
    if (Renderer::has_pipeline_statistics()) {
        encoder.begin_pipeline_statistics_query(set, index);
    }
}

void end_pipeline_statistics_query(GPU::RenderPassEncoder const& encoder)
{
    if (Renderer::has_pipeline_statistics()) {
        encoder.end_pipeline_statistics_query();
    }
}

void GBuffer::init(Vector2 const& size, GPU::BindGroupLayout const& global_bind_group_layout)
{
    GPU::TextureSpec spec {
        .label = "GBuffer::position"sv,
        .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .dimension = GPU::TextureDimension::D2,
        .size = Vector3 { size, 1 },
        .format = GPU::TextureFormat::RGBA16Float,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::All,
    };
    rt_position = Renderer::device().create_texture(spec);

    spec.label = "GBuffer::normal"sv;
    rt_normal = Renderer::device().create_texture(spec);

    spec.label = "GBuffer::albedo"sv;
    rt_albedo = Renderer::device().create_texture(spec);

    auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/GBuffer.wgsl").unwrap();

    GPU::ShaderModuleSpec shader_spec {
        .label = "GBuffer::shader"sv,
        .type = GPU::WGSLShader {
            .source = shader_src,
        },
        .vertex_entry_point = "vs_main",
        .fragment_entry_point = "fs_main",
    };

    auto shader = Renderer::device().create_shader_module(shader_spec);

    std::array entries {
        GPU::BindGroupLayoutEntry {
            .binding = 0,
            .visibility = GPU::ShaderStage::Vertex,
            .type = GPU::BindingType::Buffer {
                .type = GPU::BufferBindingType::Storage {
                    .read_only = true,
                },
                .has_dynamic_offset = false,
                .min_binding_size = None(),
            },
            .count = 1,
        },
    };

    GPU::BindGroupLayoutSpec bgl_spec {
        .label = "GBuffer::bind_group_layout"sv,
        .entries = entries,
    };

    bind_group_layout = Renderer::device().create_bind_group_layout(bgl_spec);

    std::array bind_group_layouts {
        global_bind_group_layout,
        bind_group_layout,
    };
    GPU::PipelineLayoutSpec pl_spec {
        .bind_group_layouts = bind_group_layouts
    };
    auto layout = Renderer::device().create_pipeline_layout(pl_spec);

    std::array attributes {
        GPU::VertexAttribute {
            .type = GPU::ElementType::Float3,
            .shader_location = 0,
        },
        GPU::VertexAttribute {
            .type = GPU::ElementType::Float3,
            .shader_location = 1,
        },
        GPU::VertexAttribute {
            .type = GPU::ElementType::Float4,
            .shader_location = 2,
        },
        GPU::VertexAttribute {
            .type = GPU::ElementType::Float2,
            .shader_location = 3,
        },
        GPU::VertexAttribute {
            .type = GPU::ElementType::Float3,
            .shader_location = 4,
        },
    };
    auto attribute_layout = GPU::VertexBufferLayout::create(attributes);

    GPU::RenderPipelineSpec rp_spec {
        .label = "GBuffer::pipeline"sv,
        .layout = layout,
        .vertex = {
            .attribute_layouts = { attribute_layout },
        },
        .primitive = GPU::PrimitiveState::get_default(),
        .depth_stencil = GPU::DepthStencilState::get_default(),
        .multi_sample = GPU::MultiSampleState::get_default(),
        .fragment = GPU::FragmentStage {
            .targets = {
                GPU::ColorTargetState {
                    .format = GPU::TextureFormat::RGBA16Float,
                    .blend = GPU::BlendState::get_default(),
                    .write_mask = GPU::ColorWrite::All,
                },
                GPU::ColorTargetState {
                    .format = GPU::TextureFormat::RGBA16Float,
                    .blend = GPU::BlendState::get_default(),
                    .write_mask = GPU::ColorWrite::All,
                },
                GPU::ColorTargetState {
                    .format = GPU::TextureFormat::RGBA16Float,
                    .blend = GPU::BlendState::get_default(),
                    .write_mask = GPU::ColorWrite::All,
                },
            },
        },
    };

    pipeline = Renderer::device().create_render_pipeline(shader, shader, rp_spec);
}

void GBuffer::resize(Vector2 const& new_size)
{
    rt_position.release();

    rt_normal.release();

    rt_albedo.release();

    GPU::TextureSpec spec {
        .label = "GBuffer::position"sv,
        .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .dimension = GPU::TextureDimension::D2,
        .size = Vector3 { new_size, 1 },
        .format = GPU::TextureFormat::RGBA16Float,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::All,
    };
    rt_position = Renderer::device().create_texture(spec);

    spec.label = "GBuffer::normal"sv;
    rt_normal = Renderer::device().create_texture(spec);

    spec.label = "GBuffer::albedo"sv;
    rt_albedo = Renderer::device().create_texture(spec);
}

void GBuffer::do_pass(GPU::CommandEncoder& encoder)
{
    (void)encoder;
}

void SSAO::init(Vector2 const& size, GBuffer const& gbuffer, GPU::BindGroupLayout const& global_bind_group_layout)
{
    GPU::TextureSpec spec {
        .label = "SSAO::render_target"sv,
        .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .dimension = GPU::TextureDimension::D2,
        .size = Vector3 { size, 1 },
        .format = GPU::TextureFormat::R32Float,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::All,
    };
    render_target = Renderer::device().create_texture(spec);

    GPU::TextureSpec noise_spec {
        .label = "SSAO::noise_texture"sv,
        .usage = GPU::TextureUsage::CopyDst | GPU::TextureUsage::TextureBinding,
        .dimension = GPU::TextureDimension::D2,
        .size = Vector3 { 4, 4, 1 },
        .format = SSAO_NOISE_TEXTURE_FORMAT,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::All,
    };
    noise_texture = Renderer::device().create_texture(noise_spec);

    auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/SSAO.wgsl").unwrap();

    GPU::ShaderModuleSpec shader_spec {
        .label = "SSAO::shader"sv,
        .type = GPU::WGSLShader {
            .source = shader_src,
        },
        .vertex_entry_point = "vs_main",
        .fragment_entry_point = "fs_main",
    };

    auto shader = Renderer::device().create_shader_module(shader_spec);

    std::array entries {
        GPU::BindGroupLayoutEntry {
            .binding = 0,
            .visibility = GPU::ShaderStage::Fragment,
            .type = GPU::BindingType::Texture {
                .sample_type = GPU::TextureSampleType::Float {},
                .view_dimension = GPU::TextureViewDimension::D2,
            },
            .count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .binding = 1,
            .visibility = GPU::ShaderStage::Fragment,
            .type = GPU::BindingType::Texture {
                .sample_type = GPU::TextureSampleType::Float {},
                .view_dimension = GPU::TextureViewDimension::D2,
            },
            .count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .binding = 2,
            .visibility = GPU::ShaderStage::Fragment,
            .type = GPU::BindingType::Texture {
                .sample_type = GPU::TextureSampleType::Float {
                    .filterable = true,
                },
                .view_dimension = GPU::TextureViewDimension::D2,
            },
            .count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .binding = 3,
            .visibility = GPU::ShaderStage::Fragment,
            .type = GPU::BindingType::Sampler { .type = GPU::SamplerBindingType::Filtering },
            .count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .binding = 4,
            .visibility = GPU::ShaderStage::Fragment,
            .type = GPU::BindingType::Sampler { .type = GPU::SamplerBindingType::Filtering },
            .count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .binding = 5,
            .visibility = GPU::ShaderStage::Fragment,
            .type = GPU::BindingType::Buffer {
                .type = GPU::BufferBindingType::Storage {
                    .read_only = true,
                },
                .has_dynamic_offset = false,
                .min_binding_size = None(),
            },
            .count = 1,
        },
    };

    GPU::BindGroupLayoutSpec bgl_spec {
        .label = "SSAO::bind_group_layout"sv,
        .entries = entries,
    };

    bind_group_layout = Renderer::device().create_bind_group_layout(bgl_spec);

    GPU::SamplerSpec sampler_spec {};
    sampler_spec.label = "SSAO::sampler"sv;
    sampler_spec.address_mode_u = GPU::AddressMode::ClampToEdge;
    sampler_spec.address_mode_w = GPU::AddressMode::ClampToEdge;
    sampler = Renderer::device().create_sampler(sampler_spec);

    sampler_spec = GPU::SamplerSpec {};
    sampler_spec.label = "SSAO::noise_sampler"sv,
    sampler_spec.address_mode_u = GPU::AddressMode::Repeat;
    sampler_spec.address_mode_v = GPU::AddressMode::Repeat;
    sampler_spec.address_mode_w = GPU::AddressMode::Repeat;
    noise_sampler = Renderer::device().create_sampler(sampler_spec);

    GPU::BufferSpec buffer_spec {
        .label = "SSAO::samples_buffer"sv,
        .usage = GPU::BufferUsage::Storage,
        .size = sizeof(Vector3) * 64,
        .mapped = true,
    };
    samples_buffer = Renderer::device().create_buffer(buffer_spec);

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
        sample.normalize();

        auto scale = CAST(f32, i++) / 64.0f;
        scale = Math::lerp(0.1f, 0.5f, scale * scale);
        sample *= scale;
    }

    mem::copy(samples_buffer.slice().mapped_range(), samples.data(), buffer_spec.size);

    samples_buffer.unmap();

    std::array<Vector4, 16> noise_values;
    for (auto& noise : noise_values) {
        noise = Vector4(
            random(generator) * 2.0 - 1.0,
            random(generator) * 2.0 - 1.0,
            0.0, 1.0);
    }

    Renderer::device().write_texture(
        noise_texture,
        noise_values.data(),
        16 * sizeof(Vector4),
        Vector2::Zero,
        Vector2 { 4, 4 },
        sizeof(Vector4));

    std::array bing_group_entries {
        GPU::BindGroupEntry {
            .binding = 0,
            .resource = gbuffer.rt_position.view,
        },
        GPU::BindGroupEntry {
            .binding = 1,
            .resource = gbuffer.rt_normal.view,
        },
        GPU::BindGroupEntry {
            .binding = 2,
            .resource = noise_texture.view,
        },
        GPU::BindGroupEntry {
            .binding = 3,
            .resource = sampler,
        },
        GPU::BindGroupEntry {
            .binding = 4,
            .resource = noise_sampler,
        },
        GPU::BindGroupEntry {
            .binding = 5,
            .resource = GPU::BufferBinding {
                .buffer = samples_buffer,
                .offset = 0,
                .size = buffer_spec.size,
            },
        },
    };
    GPU::BindGroupSpec bg_spec {
        .label = "SSAO::bing_group"sv,
        .entries = bing_group_entries
    };
    bind_group = Renderer::device().create_bind_group(bind_group_layout, bg_spec);

    std::array bind_group_layouts {
        global_bind_group_layout,
        bind_group_layout,
    };
    GPU::PipelineLayoutSpec pl_spec {
        .bind_group_layouts = bind_group_layouts
    };
    auto layout = Renderer::device().create_pipeline_layout(pl_spec);

    GPU::RenderPipelineSpec rp_spec {
        .label = "SSAO::pipeline"sv,
        .layout = layout,
        .vertex = {},
        .primitive = GPU::PrimitiveState::get_default(),
        .depth_stencil = None(),
        .multi_sample = GPU::MultiSampleState::get_default(),
        .fragment = GPU::FragmentStage {
            .targets = {
                GPU::ColorTargetState {
                    .format = GPU::TextureFormat::R32Float,
                    .blend = None(),
                    .write_mask = GPU::ColorWrite::All,
                },
            } },
    };

    pipeline = Renderer::device().create_render_pipeline(shader, shader, rp_spec);
}

void SSAO::resize(Vector2 const& new_size, GBuffer const& gbuffer)
{
    render_target.release();
    GPU::TextureSpec spec {
        .label = "SSAO::render_target"sv,
        .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .dimension = GPU::TextureDimension::D2,
        .size = Vector3 { new_size, 1 },
        .format = GPU::TextureFormat::R32Float,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::All,
    };
    render_target = Renderer::device().create_texture(spec);

    bind_group.release();
    std::array bing_group_entries {
        GPU::BindGroupEntry {
            .binding = 0,
            .resource = gbuffer.rt_position.view,
        },
        GPU::BindGroupEntry {
            .binding = 1,
            .resource = gbuffer.rt_normal.view,
        },
        GPU::BindGroupEntry {
            .binding = 2,
            .resource = noise_texture.view,
        },
        GPU::BindGroupEntry {
            .binding = 3,
            .resource = sampler,
        },
        GPU::BindGroupEntry {
            .binding = 4,
            .resource = noise_sampler,
        },
        GPU::BindGroupEntry {
            .binding = 5,
            .resource = GPU::BufferBinding {
                .buffer = samples_buffer,
                .offset = 0,
                .size = samples_buffer.size() },
        },
    };
    GPU::BindGroupSpec bg_spec {
        .label = "SSAO::bing_group"sv,
        .entries = bing_group_entries
    };
    bind_group = Renderer::device().create_bind_group(bind_group_layout, bg_spec);
}

void SceneRenderer::setup_scene_bind_group()
{
    std::array scene_entries {
        GPU::BindGroupLayoutEntry {
            .binding = 0,
            .visibility = GPU::ShaderStage::Fragment,
            .type = GPU::BindingType::Texture {
                .sample_type = GPU::TextureSampleType::Float {},
                .view_dimension = GPU::TextureViewDimension::D2,
                .multi_sampled = false,
            },
            .count = 1,
        },
        // Environment Map
        GPU::BindGroupLayoutEntry {
            .binding = 1,
            .visibility = GPU::ShaderStage::Fragment,
            .type = GPU::BindingType::Texture {
                .sample_type = GPU::TextureSampleType::Float {},
                .view_dimension = GPU::TextureViewDimension::Cube,
                .multi_sampled = false,
            },
            .count = 1,
        },
        GPU::BindGroupLayoutEntry {
            .binding = 2,
            .visibility = GPU::ShaderStage::Fragment,
            .type = GPU::BindingType::Sampler { .type = GPU::SamplerBindingType::Filtering },
            .count = 1,
        },
    };

    GPU::BindGroupLayoutSpec scene_bgl_spec = {
        .label = "Scene BGL"sv,
        .entries = scene_entries,
    };

    m_scene_bind_group_layout = Renderer::device().create_bind_group_layout(scene_bgl_spec);

    GPU::TextureView env_view = Renderer::white_cube_texture().view;
    if (m_render_context.environment_map != nullptr && m_environment_maps.contains(m_render_context.environment_map->handle())) {
        env_view = m_environment_maps.at(m_render_context.environment_map->handle()).view;
    }
    std::array scene_bind_group_entries = {
        GPU::BindGroupEntry {
            .binding = 0,
            .resource = ssao_blur.render_target().view,
        },
        GPU::BindGroupEntry {
            .binding = 1,
            .resource = env_view,
        },
        GPU::BindGroupEntry {
            .binding = 2,
            .resource = m_linear_sampler }
    };

    GPU::BindGroupSpec scene_bg_spec {
        .label = "Scene Bind Group"sv,
        .entries = scene_bind_group_entries
    };

    m_scene_bind_group = Renderer::device().create_bind_group(m_scene_bind_group_layout, scene_bg_spec);
}

void SceneRenderer::update_scene_bind_group(GPU::Texture const& ssao_texture)
{
    m_scene_bind_group.release();

    GPU::TextureView env_view = Renderer::white_cube_texture().view;
    if (m_render_context.environment_map != nullptr && m_environment_maps.contains(m_render_context.environment_map->handle())) {
        env_view = m_environment_maps.at(m_render_context.environment_map->handle()).view;
    }
    std::array scene_bind_group_entries = {
        GPU::BindGroupEntry {
            .binding = 0,
            .resource = ssao_texture.view,
        },
        GPU::BindGroupEntry {
            .binding = 1,
            .resource = env_view,
        },
        GPU::BindGroupEntry {
            .binding = 2,
            .resource = m_linear_sampler }
    };

    GPU::BindGroupSpec scene_bg_spec {
        .label = "Scene Bind Group"sv,
        .entries = scene_bind_group_entries
    };

    m_scene_bind_group = Renderer::device().create_bind_group(m_scene_bind_group_layout, scene_bg_spec);
}

void SceneRenderer::init()
{
    auto window_size = Application::inst()->window().size();

    create_scene_render_target(window_size);

    scene_view_data = UniformBuffer<ViewData>::create(Renderer::device(), std::string_view { "View Data" });
    scene_light_data = UniformBuffer<LightData>::create(Renderer::device(), std::string_view { "Light Data" });

    ///////////////////////
    /// BIND GROUP CREATION
    ///////////////////////

    setup_shadow_pass_render_target();

    {
        std::array entries {
            GPU::BindGroupLayoutEntry {
                .binding = 0,
                .visibility = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Buffer {
                    .type = GPU::BufferBindingType::Uniform {},
                    .has_dynamic_offset = false,
                    .min_binding_size = None(),
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 1,
                .visibility = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Buffer {
                    .type = GPU::BufferBindingType::Uniform {},
                    .has_dynamic_offset = false,
                    .min_binding_size = None(),
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 2,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture {
                    .sample_type = GPU::TextureSampleType::Depth {},
                    .view_dimension = GPU::TextureViewDimension::D2_Array,
                    .multi_sampled = false,
                },
                .count = 1,
            },
        };

        GPU::BindGroupLayoutSpec spec {
            .label = "Global BGL"sv,
            .entries = entries,
        };

        m_global_bind_group_layout = Renderer::device().create_bind_group_layout(spec);

        std::array bind_group_entries {
            GPU::BindGroupEntry {
                .binding = 0,
                .resource = GPU::BufferBinding {
                    .buffer = scene_view_data.buffer(),
                    .offset = 0,
                    .size = UniformBuffer<ViewData>::size(),
                } },
            GPU::BindGroupEntry { .binding = 1, .resource = GPU::BufferBinding {
                                                    .buffer = scene_light_data.buffer(),
                                                    .offset = 0,
                                                    .size = UniformBuffer<LightData>::size(),
                                                } },
            GPU::BindGroupEntry { .binding = 2, .resource = m_shadow_pass_render_target.view }
        };

        GPU::BindGroupSpec global_bg_spec {
            .label = "Global Bind Group"sv,
            .entries = bind_group_entries
        };

        m_global_bind_group = Renderer::device().create_bind_group(m_global_bind_group_layout, global_bg_spec);
    }

    {
        std::array entries {
            GPU::BindGroupLayoutEntry {
                .binding = 0,
                .visibility = GPU::ShaderStage::Vertex,
                .type = GPU::BindingType::Buffer {
                    .type = GPU::BufferBindingType::Storage {
                        .read_only = true,
                    },
                    .has_dynamic_offset = false,
                    .min_binding_size = None(),
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 1,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Buffer {
                    .type = GPU::BufferBindingType::Uniform {},
                    .has_dynamic_offset = false,
                    .min_binding_size = None(),
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 2,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture {
                    .sample_type = GPU::TextureSampleType::Float {},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 3,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture {
                    .sample_type = GPU::TextureSampleType::Float {},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 4,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture {
                    .sample_type = GPU::TextureSampleType::Float {},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 5,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture {
                    .sample_type = GPU::TextureSampleType::Float {},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 6,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture {
                    .sample_type = GPU::TextureSampleType::Float {},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 7,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Sampler {
                    .type = GPU::SamplerBindingType::Filtering,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry {
                .binding = 8,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Sampler {
                    .type = GPU::SamplerBindingType::Filtering,
                },
                .count = 1,
            },

        };

        GPU::BindGroupLayoutSpec spec {
            .label = "Object BGL"sv,
            .entries = entries,
        };

        m_object_bind_group_layout = Renderer::device().create_bind_group_layout(spec);
    }

    ////////////////////////
    /// RENDER PASS CREATION
    ////////////////////////

    {
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/simple.wgsl").unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .label = "Simple WGSL Shader"sv,
            .type = GPU::WGSLShader {
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts {
            m_global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec {
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        GPU::RenderPipelineSpec rp_spec {
            .label = "Simple RP"sv,
            .layout = layout,
            .vertex = {},
            .primitive = GPU::PrimitiveState::get_default(),
            .depth_stencil = None(),
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = GPU::FragmentStage {
                .targets = {
                    GPU::ColorTargetState {
                        .format = GPU::TextureFormat::RGBA8Unorm,
                        .blend = GPU::BlendState::get_default(),
                        .write_mask = GPU::ColorWrite::All,
                    } } },
        };

        m_simple_pipeline = Renderer::device().create_render_pipeline(shader, shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/Editor/Grid.wgsl").unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .label = "Grid Shader"sv,
            .type = GPU::WGSLShader {
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts {
            m_global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec {
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        GPU::RenderPipelineSpec rp_spec {
            .label = "Grid RP"sv,
            .layout = layout,
            .vertex = {},
            .primitive = GPU::PrimitiveState::get_default(),
            .depth_stencil = GPU::DepthStencilState::get_default(),
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = GPU::FragmentStage {
                .targets = {
                    GPU::ColorTargetState {
                        .format = HDRPipeline::Format,
                        .blend = GPU::BlendState::get_default(),
                        .write_mask = GPU::ColorWrite::All,
                    } } },
        };

        m_grid_pipeline = Renderer::device().create_render_pipeline(shader, shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/Sky.wgsl").unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .label = "Sky Shader:VS"sv,
            .type = GPU::WGSLShader {
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts {
            m_global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec {
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        auto primitive = GPU::PrimitiveState::get_default();
        primitive.topology = GPU::PrimitiveTopology::TriangleStrip;

        auto depth = GPU::DepthStencilState::get_default();
        depth.depth_write_enabled = false;
        depth.depth_compare = GPU::CompareFunction::Always;
        GPU::RenderPipelineSpec rp_spec {
            .label = "Sky RP"sv,
            .layout = layout,
            .vertex = {},
            .primitive = primitive,
            .depth_stencil = depth,
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = GPU::FragmentStage {
                .targets = {
                    GPU::ColorTargetState {
                        .format = HDRPipeline::Format,
                        .blend = GPU::BlendState::get_default(),
                        .write_mask = GPU::ColorWrite::All,
                    },
                },
            },
        };

        m_sky_pipeline = Renderer::device().create_render_pipeline(shader, shader, rp_spec);
    }

    GPU::BufferSpec ibs {
        .label = "PBR Instance Buffer"sv,
        .usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        .size = sizeof(Mat4) * 2'000,
        .mapped = false,
    };
    m_pbr_instance_buffer = Renderer::device().create_buffer(ibs);

    m_pbr_instance_staging_buffer.reserve(sizeof(Mat4) * 2'000);

    GPU::SamplerSpec bilinear_sampler_spec {
        .label = "Bilinear Sampler"sv,
        .address_mode_u = GPU::AddressMode::Repeat,
        .address_mode_v = GPU::AddressMode::Repeat,
        .address_mode_w = GPU::AddressMode::Repeat,
        .mag_filter = GPU::FilterMode::Linear,
        .min_filter = GPU::FilterMode::Linear,
        .mip_map_filter = GPU::FilterMode::Linear,
        .lod_min_clamp = 0.f,
        .lod_max_clamp = 32.f,
        .anisotropy_clamp = 16
    };

    m_linear_sampler = Renderer::device().create_sampler(bilinear_sampler_spec);
    bilinear_sampler_spec.mag_filter = GPU::FilterMode::Linear;
    bilinear_sampler_spec.min_filter = GPU::FilterMode::Linear;
    // bilinear_sampler_spec.anisotropy_clamp = 1_u16;

    m_shadow_sampler = Renderer::device().create_sampler(bilinear_sampler_spec);

    m_hdr_pipeline.init(window_size, m_scene_render_target.spec.format);

    gbuffer.init(window_size, m_global_bind_group_layout);
    ssao.init(window_size, gbuffer, m_global_bind_group_layout);
    ssao_blur.init(window_size);

    setup_scene_bind_group();

    // Creating the pbr pipeline after the scene bind group, which must be done after the ssao blur pipeline. oof incarnate.
    {
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/PBR.wgsl").unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .label = "PBR Shader"sv,
            .type = GPU::WGSLShader {
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts {
            m_global_bind_group_layout,
            m_scene_bind_group_layout,
            m_object_bind_group_layout,
        };
        GPU::PipelineLayoutSpec pl_spec {
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        std::array attributes {
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float3,
                .shader_location = 0,
            },
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float3,
                .shader_location = 1,
            },
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float4,
                .shader_location = 2,
            },
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float2,
                .shader_location = 3,
            },
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float3,
                .shader_location = 4,
            },
        };
        auto attribute_layout = GPU::VertexBufferLayout::create(attributes);

        GPU::RenderPipelineSpec rp_spec {
            .label = "PBR Render Pass"sv,
            .layout = layout,
            .vertex = {
                .attribute_layouts = { attribute_layout } },
            .primitive = {
                .topology = GPU::PrimitiveTopology::TriangleList,
                .strip_index_format = None(),
                .front_face = GPU::FrontFace::Ccw,
                .cull = GPU::Face::None,
            },
            .depth_stencil = GPU::DepthStencilState::get_default(),
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = GPU::FragmentStage {
                .targets = {
                    GPU::ColorTargetState {
                        .format = HDRPipeline::Format,
                        .blend = GPU::BlendState::get_default(),
                        .write_mask = GPU::ColorWrite::All,
                    },
                },
            },
        };

        m_pbr_pipeline = Renderer::device().create_render_pipeline(shader, shader, rp_spec);
    }

    m_cube_skybox.init({ m_global_bind_group_layout, m_scene_bind_group_layout });

    Debug::initialize(Renderer::device(), m_global_bind_group_layout, m_hdr_pipeline.Format);

    setup_shadow_pass();

    setup_queries();
}

void SceneRenderer::resize(Vector2 const& new_size)
{
    ZoneScoped;
    m_render_area = new_size;

    m_scene_render_target.release();
    m_scene_render_depth_target.release();
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
    auto log = near * Math::pow(far / near, split_ratio);
    auto uniform = near + (far - near) * split_ratio;
    auto d = l * (log - uniform) + uniform;
    return (d - near) / (far - near);
}

void SceneRenderer::render(GPU::CommandEncoder& encoder, RenderPacket const& packet, bool game_view)
{
    ZoneScoped;

    if (Renderer::has_pipeline_statistics()) {
        if (m_timings_read_buffer.map_state() == GPU::MapState::Unmapped) {
            m_timings_read_buffer.slice().map_async(GPU::MapMode::Read, [this] {
                auto data = CAST(u64*, m_timings_read_buffer.slice().mapped_range());
                timings.gbuffer = CAST(f64, data[3] - data[2]) * 1e-6;
                timings.ssao = CAST(f64, data[5] - data[4]) * 1e-6;
                timings.ssao_blur = CAST(f64, data[7] - data[6]) * 1e-6;
                timings.pbr = CAST(f64, data[9] - data[8]) * 1e-6;

                m_timings_read_buffer.unmap();
            });
        }

        if (m_statistics_read_buffer.map_state() == GPU::MapState::Unmapped) {
            m_statistics_read_buffer.slice().map_async(GPU::MapMode::Read, [this] {
                auto data = CAST(u64*, m_statistics_read_buffer.slice().mapped_range());
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

                m_statistics_read_buffer.unmap();
            });
        }
    }

    m_render_context.reset();

    {
        ZoneScopedN("Render Object Collection");
        m_render_context.render_flags = RenderState::LightCollection;
        if (packet.scene) {
            packet.scene->for_each_entity([&](Entity* entity) {
                entity->on_draw(m_render_context);
            });
        }
    }

    scene_view_data.data.perspective = packet.camera.perspective;
    scene_view_data.data.view = packet.camera.view;
    scene_view_data.data.view_rotation_only = packet.camera.rotation;
    scene_view_data.data.position = packet.camera.position;
    scene_view_data.data.screen_size = m_render_area;
    scene_view_data.flush();

    if (!m_render_context.directional_lights.empty()) {
        scene_light_data.data.directional_light = m_render_context.directional_lights[0].shader_data;
        scene_light_data.data.shadow_split_distances = Vector4 { 0.0f };
    }
    scene_light_data.flush();

    depth_pass(encoder, packet);
    if (m_render_context.environment_map != nullptr) {
        if (auto handle = m_render_context.environment_map->handle(); !m_environment_maps.contains(handle)) {
            IBLIrradiance ibl_irradiance {};
            ibl_irradiance.init();
            auto texture = ibl_irradiance.generate(m_render_context.environment_map->image());
            m_environment_maps[handle] = texture;
            // Somehow this causes a User-mode data execution prevention (DEP) violation
            // m_environment_maps[handle] = gen.generate(m_render_context.environment_map->image());
        }
    }
    pbr_pass(encoder, packet, game_view);

    m_hdr_pipeline.process(encoder, m_scene_render_target.view);

    for (auto& group : m_object_groups_to_release) {
        group.release();
    }
    m_object_groups_to_release.clear();

    Debug::reset();
}

void SceneRenderer::setup_shadow_pass_render_target()
{
    GPU::TextureSpec spec {
        .label = "DepthPass::RenderTarget"sv,
        .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .dimension = GPU::TextureDimension::D2,
        .size = { SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION, MAX_SHADOW_CASCADES },
        .format = GPU::TextureFormat::Depth24Plus,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::DepthOnly,
    };
    m_shadow_pass_render_target = Renderer::device().create_texture(spec);
    m_shadow_pass_render_target.initialize_view(MAX_SHADOW_CASCADES);

    for (u32 i = 0; i < MAX_SHADOW_CASCADES; ++i) {
        m_shadow_pass_render_target_views[i] = m_shadow_pass_render_target.create_view({
            .label = "Shadow Pass Render Target"sv,
            .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
            .dimension = GPU::TextureViewDimension::D2,
            .format = GPU::TextureFormat::Depth24Plus,
            .base_mip_level = 0,
            .mip_level_count = 1,
            .base_array_layer = i,
            .array_layer_count = 1,
            .aspect = GPU::TextureAspect::DepthOnly,
        });
    }
}

void SceneRenderer::setup_shadow_pass()
{
    {
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/DepthPass.wgsl").unwrap();

        GPU::ShaderModuleSpec shader_spec {
            .label = "PBR Shader"sv,
            .type = GPU::WGSLShader {
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            //.FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bgl_entries {
            GPU::BindGroupLayoutEntry {
                .binding = 0,
                .visibility = GPU::ShaderStage::Vertex,
                .type = GPU::BindingType::Buffer {
                    .type = GPU::BufferBindingType::Storage {
                        .read_only = true,
                    },
                },
                .count = 1,
            },
        };
        GPU::BindGroupLayoutSpec bgl_spec {
            .entries = bgl_entries,
        };

        m_object_depth_bgl = Renderer::device().create_bind_group_layout(bgl_spec);

        std::array bind_group_layouts {
            m_object_depth_bgl,
        };
        GPU::PipelineLayoutSpec pl_spec {
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        std::array attributes {
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float3,
                .shader_location = 0,
            },
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float3,
                .shader_location = 1,
            },
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float4,
                .shader_location = 2,
            },
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float2,
                .shader_location = 3,
            },
            GPU::VertexAttribute {
                .type = GPU::ElementType::Float3,
                .shader_location = 4,
            },
        };
        auto attribute_layout = GPU::VertexBufferLayout::create(attributes);

        GPU::RenderPipelineSpec rp_spec {
            .label = "DepthPass::Pipeline"sv,
            .layout = layout,
            .vertex = {
                .attribute_layouts = { attribute_layout } },
            .primitive = {
                .topology = GPU::PrimitiveTopology::TriangleList,
                .strip_index_format = None(),
                .front_face = GPU::FrontFace::Ccw,
                .cull = GPU::Face::None,
            },
            .depth_stencil = GPU::DepthStencilState::get_default(),
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = None(),
        };

        m_depth_pipeline = Renderer::device().create_render_pipeline(shader, shader, rp_spec);
    }

    GPU::BufferSpec ibs {
        .label = "Depth Instance Buffer"sv,
        .usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        .size = sizeof(Mat4) * 2'000,
        .mapped = false,
    };
    m_depth_instance_buffer = Renderer::device().create_buffer(ibs);

    m_depth_instance_staging_buffer.reserve(sizeof(Mat4) * 2'000);
}

void SceneRenderer::depth_pass(GPU::CommandEncoder& encoder, RenderPacket const& packet)
{
    {
        ZoneScopedN("Depth Pass");

        for (auto& light : m_render_context.directional_lights) {

            std::array<f32, MAX_SHADOW_CASCADES> shadow_splits {};
            for (auto i = 0; i < MAX_SHADOW_CASCADES; i++) {
                // SceneLightData.Data.ShadowSplitDistances[i] = GetSplitDepth(i + 1, ShadowCascades, packet.Camera.Near, packet.Camera.Far, light.Split);
                shadow_splits[i] = GetSplitDepth(i + 1, MAX_SHADOW_CASCADES, packet.camera.near, packet.camera.far, light.split);
            }

            GPU::Buffer instance_buffer;
            if (!m_instance_buffer_pool.empty()) {
                instance_buffer = m_instance_buffer_pool.back();
                m_instance_buffer_pool.pop_back();
            } else {
                GPU::BufferSpec spec {
                    .label = "Depth Instance Buffer"sv,
                    .usage = GPU::BufferUsage::CopySrc | GPU::BufferUsage::MapWrite,
                    .size = sizeof(Mat4) * 2 * 2'000,
                    .mapped = true,
                };
                instance_buffer = Renderer::device().create_buffer(spec);
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
                    f32 distance = (frustum_corner - center).length();
                    radius = Math::max(radius, distance);
                }
                radius = std::ceil(radius * 16.f) / 16.f;

                auto texel_size = CAST(f32, SHADOWMAP_RESOLUTION) / (radius * 2.0f);
                Mat4 scalar(texel_size);

                Vector3 max_extents { radius, radius, radius };
                Vector3 min_extents = -max_extents;

                glm::mat4 view = glm::lookAt(glm::vec3(center - Vector3(light.shader_data.direction) * min_extents.z), glm::vec3(center), glm::vec3(Vector3::Up));
                view = scalar * view;
                center = glm::mat3(view) * center;
                center.x = Math::floor(center.x);
                center.y = Math::floor(center.y);
                center = inverse(glm::mat3(view)) * center;
                view = glm::lookAt(glm::vec3(center - Vector3(light.shader_data.direction) * min_extents.z), glm::vec3(center), glm::vec3(Vector3::Up));

                glm::mat4 proj = glm::ortho(min_extents.x, max_extents.x, min_extents.y, max_extents.y, -20.0f, max_extents.z - min_extents.z);

                light.shader_data.light_space_matrix[i] = proj * view;

                scene_light_data.data.shadow_split_distances[i] = packet.camera.near + split * (packet.camera.far - packet.camera.near) * -1.0f;
                scene_light_data.data.directional_light.light_space_matrix[i] = light.shader_data.light_space_matrix[i];

                for (auto const& [material, mesh_map] : m_render_context.mesh_render_lists) {
                    (void)material;
                    for (auto const& [buffer, list] : mesh_map) {
                        (void)buffer;
                        auto data = TRANSMUTE(DepthInstanceData*, instance_buffer.slice().mapped_range()) + buffer_offset;
                        int j = 0;
                        for (auto index : list) {
                            auto& obj = m_render_context.render_objects[index];
                            data[j].model = obj.world_matrix;
                            data[j++].light_space = light.shader_data.light_space_matrix[i];
                        }

                        buffer_offset += list.size();
                    }
                }
                last_split = split;
            }

            instance_buffer.unmap();
            auto copy_encoder = Renderer::device().create_command_encoder();

            copy_encoder.copy_buffer_to_buffer(instance_buffer, 0, m_depth_instance_buffer, 0, buffer_offset * sizeof(DepthInstanceData));
            Renderer::device().submit_command_buffer(copy_encoder.finish());
            copy_encoder.release();

            instance_buffer.slice().map_async(GPU::MapMode::Write, [instance_buffer, this] {
                auto& copy = m_instance_buffer_pool.emplace_back(instance_buffer);
                copy.force_map_state(GPU::MapState::Mapped);
            });

            buffer_offset = 0;
            for (auto i = 0; i < MAX_SHADOW_CASCADES; i++) {
                GPU::RenderPassSpec rp_spec {
                    .label = "DepthPass::RenderPass"sv,
                    .depth_stencil_attachment = GPU::RenderPassColorAttachment {
                        .view = m_shadow_pass_render_target_views[i],
                        .load_op = GPU::LoadOp::Clear,
                        .store_op = GPU::StoreOp::Store,
                        .depth_clear = 1.0f,
                    },
                };
                auto rp = encoder.begin_rendering(rp_spec);
                rp.set_viewport({}, { SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION });
                rp.set_pipeline(m_depth_pipeline);

                std::array bind_group_entries {
                    GPU::BindGroupEntry {
                        .binding = 0,
                        .resource = GPU::BufferBinding {
                            .buffer = m_depth_instance_buffer,
                            .offset = 0,
                            .size = m_depth_instance_buffer.size(),
                        } },
                };

                GPU::BindGroupSpec bg_spec {
                    .label = "Object Depth Bind Group"sv,
                    .entries = bind_group_entries
                };

                auto object_group = Renderer::device().create_bind_group(m_object_depth_bgl, bg_spec);
                m_object_groups_to_release.push_back(object_group);
                rp.set_bind_group(object_group, 0);
                for (auto const& [material, mesh_map] : m_render_context.mesh_render_lists) {

                    for (auto const& [buffer, list] : mesh_map) {
                        if (list.empty())
                            continue;
                        (void)buffer;

                        auto& hack = m_render_context.render_objects[list[0]];
                        rp.set_vertex_buffer(0, hack.vertex_buffer);
                        rp.set_index_buffer(hack.index_buffer);

                        rp.draw_index({ 0, hack.index_count }, { CAST(u32, buffer_offset), CAST(u32, list.size()) });
                        buffer_offset += list.size();
                    }
                }
                rp.end();
                rp.release();
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
            .label = "PBR Instance Buffer"sv,
            .usage = GPU::BufferUsage::CopySrc | GPU::BufferUsage::MapWrite,
            .size = sizeof(Mat4) * 2'000,
            .mapped = true,
        };
        instance_buffer = Renderer::device().create_buffer(spec);
    }

    usz buffer_count_offset = 0;

    for (auto const& [material, mesh_map] : m_render_context.mesh_render_lists) {
        for (auto const& [buffer, list] : mesh_map) {
            VERIFY(instance_buffer.map_state() == GPU::MapState::Mapped, "{}", magic_enum::enum_name(instance_buffer.map_state()));
            auto data = TRANSMUTE(InstanceData*, instance_buffer.slice().mapped_range()) + buffer_count_offset;
            int j = 0;
            for (auto index : list) {
                auto& obj = m_render_context.render_objects[index];
                data[j++].model = obj.world_matrix;
            }

            buffer_count_offset += list.size();
        }
    }
    instance_buffer.unmap();

    auto copy_encoder = Renderer::device().create_command_encoder();

    copy_encoder.copy_buffer_to_buffer(instance_buffer, 0, m_pbr_instance_buffer, 0, buffer_count_offset * sizeof(InstanceData));
    Renderer::device().submit_command_buffer(copy_encoder.finish());
    copy_encoder.release();

    instance_buffer.slice().map_async(GPU::MapMode::Write, [instance_buffer, this] {
        auto& copy = m_instance_buffer_pool.emplace_back(instance_buffer);
        copy.force_map_state(GPU::MapState::Mapped);
    });

    {
        ZoneScopedN("G-Buffer");
        std::array color_attachments {
            GPU::RenderPassColorAttachment {
                .view = gbuffer.rt_position.view,
                .load_op = GPU::LoadOp::Clear,
                .store_op = GPU::StoreOp::Store,
                .clear_color = Color::Black,
            },
            GPU::RenderPassColorAttachment {
                .view = gbuffer.rt_normal.view,
                .load_op = GPU::LoadOp::Clear,
                .store_op = GPU::StoreOp::Store,
                .clear_color = Color::Black,
            },
            GPU::RenderPassColorAttachment {
                .view = gbuffer.rt_albedo.view,
                .load_op = GPU::LoadOp::Clear,
                .store_op = GPU::StoreOp::Store,
                .clear_color = Color::Black,
            },
        };

        GPU::RenderPassTimestampWrites timestamp_writes {
            .query_set = m_timings_set,
            .beginning_of_pass_write_index = 2,
            .end_of_pass_write_index = 3
        };
        GPU::RenderPassSpec gpass_rp_spec {
            .label = "GBuffer::render_pass"sv,
            .color_attachments = color_attachments,
            .depth_stencil_attachment = GPU::RenderPassColorAttachment {
                .view = m_scene_render_depth_target.view,
                .load_op = GPU::LoadOp::Clear,
                .store_op = GPU::StoreOp::Store,
                .depth_clear = 1.0f,
            },
            .timestamp_writes = timestamp_writes,
        };
        auto gpass = encoder.begin_rendering(gpass_rp_spec);
        gpass.set_pipeline(gbuffer.pipeline);
        gpass.set_bind_group(m_global_bind_group, 0);
        begin_pipeline_statistics_query(gpass, m_statistics_query_set, 0);

        std::array bind_group_entries {
            GPU::BindGroupEntry {
                .binding = 0,
                .resource = GPU::BufferBinding {
                    .buffer = m_pbr_instance_buffer,
                    .offset = 0,
                    .size = m_pbr_instance_buffer.size(),
                } },
        };

        GPU::BindGroupSpec bg_spec {
            .label = "Object Bind Group"sv,
            .entries = bind_group_entries
        };

        auto object_group = Renderer::device().create_bind_group(gbuffer.bind_group_layout, bg_spec);
        m_object_groups_to_release.push_back(object_group);
        gpass.set_bind_group(object_group, 1);
        buffer_count_offset = 0;
        for (auto const& [material, mesh_map] : m_render_context.mesh_render_lists) {

            for (auto& [buffer, list] : mesh_map) {
                if (list.empty())
                    continue;
                (void)buffer;
                auto& hack = m_render_context.render_objects[list[0]];
                gpass.set_vertex_buffer(0, hack.vertex_buffer);
                gpass.set_index_buffer(hack.index_buffer);

                gpass.draw_index({ 0, hack.index_count }, { CAST(u32, buffer_count_offset), CAST(u32, list.size()) });
                buffer_count_offset += list.size();
            }
        }

        end_pipeline_statistics_query(gpass);
        gpass.end();
        gpass.release();
    }

    {
        ZoneScopedN("SSAO");
        std::array color_attachments {
            GPU::RenderPassColorAttachment {
                .view = ssao.render_target.view,
                .load_op = GPU::LoadOp::Clear,
                .store_op = GPU::StoreOp::Store,
                .clear_color = Color::White,
            },
        };

        GPU::RenderPassTimestampWrites timestamp_writes {
            .query_set = m_timings_set,
            .beginning_of_pass_write_index = 4,
            .end_of_pass_write_index = 5
        };

        GPU::RenderPassSpec rp_spec {
            .label = "SSAO::render_pass"sv,
            .color_attachments = color_attachments,
            .timestamp_writes = timestamp_writes
        };
        auto pass = encoder.begin_rendering(rp_spec);
        begin_pipeline_statistics_query(pass, m_statistics_query_set, 1);

        if (m_render_context.post_processing.use_ssao) {
            pass.set_pipeline(ssao.pipeline);
            pass.set_bind_group(m_global_bind_group, 0);
            pass.set_bind_group(ssao.bind_group, 1);
            pass.draw({ 0, 3 }, { 0, 1 });
        }

        end_pipeline_statistics_query(pass);
        pass.end();
        pass.release();
    }

    ssao_blur.draw(encoder, m_timings_set, 6, 7);

    {
        ZoneScopedN("PBR Pass");
        std::array color_attachments {
            GPU::RenderPassColorAttachment {
                .view = m_hdr_pipeline.view(),
                .load_op = GPU::LoadOp::Clear,
                .store_op = GPU::StoreOp::Store,
                .clear_color = Color::Black,
            },
        };

        GPU::RenderPassSpec scene_rp_spec {
            .label = "Scene Render Pass"sv,
            .color_attachments = color_attachments,
            .depth_stencil_attachment = GPU::RenderPassColorAttachment {
                .view = m_scene_render_depth_target.view,
                .load_op = GPU::LoadOp::Clear,
                .store_op = GPU::StoreOp::Store,
                .depth_clear = 1.0f,
            },
            .timestamp_writes = GPU::RenderPassTimestampWrites { .query_set = m_timings_set, .beginning_of_pass_write_index = 8, .end_of_pass_write_index = 9 }
        };
        auto scene_rp = encoder.begin_rendering(scene_rp_spec);
        begin_pipeline_statistics_query(scene_rp, m_statistics_query_set, 2);

        scene_rp.set_viewport(Vector2::Zero, { m_render_area.x, m_render_area.y });

        scene_rp.set_bind_group(m_global_bind_group, 0);
        scene_rp.set_bind_group(m_scene_bind_group, 1);
        {
            scene_rp.set_pipeline(m_sky_pipeline);
            scene_rp.draw({ 0, 4 }, { 0, 1 });
        }

        m_cube_skybox.execute(scene_rp);

        scene_rp.set_pipeline(m_pbr_pipeline);

        // For each material
        // - Create a bind group
        // - Bind it
        // - For each mesh list
        // -- Draw it

        buffer_count_offset = 0;
        for (auto const& [material, mesh_map] : m_render_context.mesh_render_lists) {

            material->update_buffer();

            auto albedo = material->albedo_map.get();
            if (!albedo) {
                albedo = Renderer::white_texture().get();
            }

            auto normal = material->normal_map.get();
            if (!normal) {
                normal = Renderer::default_normal_map().get();
            }

            auto ao = material->ambient_occlusion_map.get();
            if (!ao) {
                ao = Renderer::white_texture().get();
            }

            auto metallic_roughness = material->metallic_roughness_map.get();
            if (!metallic_roughness) {
                metallic_roughness = Renderer::white_texture().get();
            }

            auto emissive = material->emissive_map.get();
            if (!emissive) {
                emissive = Renderer::black_texture().get();
            }

            std::array bind_group_entries {
                GPU::BindGroupEntry {
                    .binding = 0,
                    .resource = GPU::BufferBinding {
                        .buffer = m_pbr_instance_buffer,
                        .offset = 0,
                        .size = m_pbr_instance_buffer.size(),
                    },
                },
                GPU::BindGroupEntry {
                    .binding = 1,
                    .resource = GPU::BufferBinding {
                        .buffer = material->material_uniform_buffer.buffer(),
                        .offset = 0,
                        .size = material->material_uniform_buffer.buffer().size(),
                    },
                },
                GPU::BindGroupEntry {
                    .binding = 2,
                    .resource = albedo->image().view,
                },
                GPU::BindGroupEntry {
                    .binding = 3,
                    .resource = normal->image().view,
                },
                GPU::BindGroupEntry {
                    .binding = 4,
                    .resource = metallic_roughness->image().view,
                },
                GPU::BindGroupEntry {
                    .binding = 5,
                    .resource = ao->image().view,
                },
                GPU::BindGroupEntry {
                    .binding = 6,
                    .resource = emissive->image().view,
                },
                GPU::BindGroupEntry {
                    .binding = 7,
                    .resource = m_linear_sampler,
                },
                GPU::BindGroupEntry {
                    .binding = 8,
                    .resource = m_shadow_sampler,
                },
            };

            GPU::BindGroupSpec global_bg_spec {
                .label = "Object Bind Group"sv,
                .entries = bind_group_entries
            };

            auto object_group = Renderer::device().create_bind_group(m_object_bind_group_layout, global_bg_spec);
            m_object_groups_to_release.push_back(object_group);
            scene_rp.set_bind_group(object_group, 2);

            for (auto const& [buffer, list] : mesh_map) {
                if (list.empty())
                    continue;
                (void)buffer;
                auto& hack = m_render_context.render_objects[list[0]];

                scene_rp.set_vertex_buffer(0, hack.vertex_buffer);
                scene_rp.set_index_buffer(hack.index_buffer);

                scene_rp.draw_index({ 0, hack.index_count }, { CAST(u32, buffer_count_offset), CAST(u32, list.size()) });
                buffer_count_offset += list.size();
            }
        }
        // Draw editor specific stuff only if we are not rendering a game view.
        if (!game_view) {
            Debug::render(scene_rp);

            scene_rp.set_pipeline(m_grid_pipeline);
            scene_rp.draw({ 0, 6 }, { 0, 1 });
        }

        end_pipeline_statistics_query(scene_rp);
        scene_rp.end();
        scene_rp.release();
    }

    if (Renderer::has_pipeline_statistics()) {
        encoder.resolve_query_set(m_timings_set, { 0, 10 }, m_timings_resolve_buffer, 0);
        encoder.resolve_query_set(m_statistics_query_set, { 0, 3 }, m_statistics_resolve_buffer, 0);

        if (m_timings_read_buffer.map_state() == GPU::MapState::Unmapped) {
            encoder.copy_buffer_to_buffer(m_timings_resolve_buffer, 0, m_timings_read_buffer, 0, m_timings_read_buffer.size());
        }
        if (m_statistics_read_buffer.map_state() == GPU::MapState::Unmapped) {
            encoder.copy_buffer_to_buffer(m_statistics_resolve_buffer, 0, m_statistics_read_buffer, 0, m_statistics_read_buffer.size());
        }
    }
}

void SceneRenderer::setup_queries()
{

    {
        GPU::QuerySetSpec query_set_spec {
            .label = String("Timings Set"),
            .type = GPU::QueryType::Timestamp {},
            .count = 5 * 2, // 5, one for each pass, times 2 for start and end
        };

        m_timings_set = Renderer::device().create_query_set(query_set_spec);

        GPU::BufferSpec resolve_spec {
            .label = "Timings Resolve Buffer"sv,
            .usage = GPU::BufferUsage::QueryResolve | GPU::BufferUsage::CopySrc,
            .size = query_set_spec.count * 8, // Each element in a querySet takes 8 bytes: https://webgpufundamentals.org/webgpu/lessons/webgpu-timing.html
            .mapped = false
        };

        m_timings_resolve_buffer = Renderer::device().create_buffer(resolve_spec);

        GPU::BufferSpec read_spec {
            .label = "Timings Read Buffer"sv,
            .usage = GPU::BufferUsage::MapRead | GPU::BufferUsage::CopyDst,
            .size = resolve_spec.size,
            .mapped = false
        };

        m_timings_read_buffer = Renderer::device().create_buffer(read_spec);
    }

    if (Renderer::has_pipeline_statistics()) {
        using enum GPU::PipelineStatisticName;
        GPU::QuerySetSpec query_set_spec {
            .label = String("Pipeline Statistics Set"),
            .type = FragmentShaderInvocations | VertexShaderInvocations | ClipperInvocations,
            .count = 4 * 3, // 5, one for each pass, times 3, for each of the types above
        };

        m_statistics_query_set = Renderer::device().create_query_set(query_set_spec);

        GPU::BufferSpec resolve_spec {
            .label = "Pipeline Statistics Resolve Buffer"sv,
            .usage = GPU::BufferUsage::QueryResolve | GPU::BufferUsage::CopySrc,
            .size = query_set_spec.count * 8, // Each element in a querySet takes 8 bytes: https://webgpufundamentals.org/webgpu/lessons/webgpu-timing.html
            .mapped = false
        };

        m_statistics_resolve_buffer = Renderer::device().create_buffer(resolve_spec);

        GPU::BufferSpec read_spec {
            .label = "Pipeline Statistics Read Buffer"sv,
            .usage = GPU::BufferUsage::MapRead | GPU::BufferUsage::CopyDst,
            .size = resolve_spec.size,
            .mapped = false
        };

        m_statistics_read_buffer = Renderer::device().create_buffer(read_spec);
    }
}

void SceneRenderer::create_scene_render_target(Vector2 const& size)
{
    GPU::TextureSpec spec {
        .label = "Main Render Target"sv,
        .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .dimension = GPU::TextureDimension::D2,
        .size = Vector3 { size, 1 },
        .format = GPU::TextureFormat::RGBA8Unorm,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::All,
    };
    m_scene_render_target = Renderer::device().create_texture(spec);

    GPU::TextureSpec depth_spec {
        .label = "Main Render Depth Target"sv,
        .usage = GPU::TextureUsage::RenderAttachment,
        .dimension = GPU::TextureDimension::D2,
        .size = Vector3 { size, 1 },
        .format = GPU::TextureFormat::Depth24Plus,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::DepthOnly,
    };
    m_scene_render_depth_target = Renderer::device().create_texture(depth_spec);
}
