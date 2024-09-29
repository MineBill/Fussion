#include "FussionPCH.h"
#include "GPU/ShaderProcessor.h"
#include "GPU/Utils.h"
#include "IBLIrradiance.h"
#include "Rendering/Renderer.h"
#include "tracy/Tracy.hpp"

namespace Fussion {

    static std::vector CUBE_VERTICES {
        // // Front face
        Vector3(-0.5f, -0.5f, 0.5f), // Bottom-left
        Vector3(0.5f, -0.5f, 0.5f),  // Bottom-right
        Vector3(0.5f, 0.5f, 0.5f),   // Top-right
        Vector3(-0.5f, -0.5f, 0.5f), // Bottom-left
        Vector3(0.5f, 0.5f, 0.5f),   // Top-right
        Vector3(-0.5f, 0.5f, 0.5f),  // Top-left
        // Back face
        Vector3(-0.5f, -0.5f, -0.5f), // Bottom-left
        Vector3(0.5f, -0.5f, -0.5f),  // Bottom-right
        Vector3(0.5f, 0.5f, -0.5f),   // Top-right
        Vector3(-0.5f, -0.5f, -0.5f), // Bottom-left
        Vector3(0.5f, 0.5f, -0.5f),   // Top-right
        Vector3(-0.5f, 0.5f, -0.5f),  // Top-left
        // Left face
        Vector3(-0.5f, -0.5f, -0.5f), // Bottom-left
        Vector3(-0.5f, -0.5f, 0.5f),  // Bottom-right
        Vector3(-0.5f, 0.5f, 0.5f),   // Top-right
        Vector3(-0.5f, -0.5f, -0.5f), // Bottom-left
        Vector3(-0.5f, 0.5f, 0.5f),   // Top-right
        Vector3(-0.5f, 0.5f, -0.5f),  // Top-left
        // Right face
        Vector3(0.5f, -0.5f, -0.5f), // Bottom-left
        Vector3(0.5f, -0.5f, 0.5f),  // Bottom-right
        Vector3(0.5f, 0.5f, 0.5f),   // Top-right
        Vector3(0.5f, -0.5f, -0.5f), // Bottom-left
        Vector3(0.5f, 0.5f, 0.5f),   // Top-right
        Vector3(0.5f, 0.5f, -0.5f),  // Top-left
        // Top face
        Vector3(-0.5f, 0.5f, -0.5f), // Bottom-left
        Vector3(0.5f, 0.5f, -0.5f),  // Bottom-right
        Vector3(0.5f, 0.5f, 0.5f),   // Top-right
        Vector3(-0.5f, 0.5f, -0.5f), // Bottom-left
        Vector3(0.5f, 0.5f, 0.5f),   // Top-right
        Vector3(-0.5f, 0.5f, 0.5f),  // Top-left
        // Bottom face
        Vector3(-0.5f, -0.5f, -0.5f), // Bottom-left
        Vector3(0.5f, -0.5f, -0.5f),  // Bottom-right
        Vector3(0.5f, -0.5f, 0.5f),   // Top-right
        Vector3(-0.5f, -0.5f, -0.5f), // Bottom-left
        Vector3(0.5f, -0.5f, 0.5f),   // Top-right
        Vector3(-0.5f, -0.5f, 0.5f),  // Top-left
    };

    IBLIrradiance::~IBLIrradiance()
    {
        m_bind_group.release();
    }

    void IBLIrradiance::init()
    {
        ZoneScoped;
        using namespace GPU;
        {
            auto shader_src = ShaderProcessor::process_file(
                "Assets/Shaders/WGSL/OneShot/CubeTexGen.wgsl")
                                  .unwrap();

            ShaderModuleSpec shader_spec {
                .label = "CubeTexGen::Shader",
                .type = WGSLShader {
                    .source = shader_src,
                },
                .vertex_entry_point = "vs_main",
                .fragment_entry_point = "fs_main",
            };

            m_shader = Renderer::device().create_shader_module(shader_spec);

            std::vector bgl_entries {
                BindGroupLayoutEntry {
                    .binding = 0,
                    .visibility = ShaderStage::Vertex,
                    .type = BindingType::Buffer {
                        .type = BufferBindingType::Uniform {},
                    },
                    .count = 1,
                },
                BindGroupLayoutEntry {
                    .binding = 1,
                    .visibility = ShaderStage::Fragment,
                    .type = BindingType::Texture {
                        .sample_type = TextureSampleType::Float {},
                        .view_dimension = TextureViewDimension::D2,
                        .multi_sampled = false,
                    },
                    .count = 1,
                },
                BindGroupLayoutEntry {
                    .binding = 2,
                    .visibility = ShaderStage::Fragment,
                    .type = BindingType::Sampler {
                        .type = SamplerBindingType::Filtering,
                    },
                    .count = 1,
                },

            };
            BindGroupLayoutSpec bgl_spec {
                .label = "CubeTexGen::BGL",
                .entries = bgl_entries
            };
            m_bind_group_layout = Renderer::device().create_bind_group_layout(bgl_spec);

            std::array layouts = {
                m_bind_group_layout
            };

            PipelineLayoutSpec layout_spec {
                .label = "CubeTexGen::PipelineLayout",
                .bind_group_layouts = layouts,
            };
            auto layout = Renderer::device().create_pipeline_layout(layout_spec);

            std::array attributes {
                VertexAttribute {
                    .type = ElementType::Float3,
                    .shader_location = 0,
                },
            };
            auto attribute_layout = VertexBufferLayout::create(attributes);

            auto depth = DepthStencilState::get_default();
            depth.depth_write_enabled = false;
            depth.depth_compare = CompareFunction::Always;

            RenderPipelineSpec spec {
                .label = "CubeTexGen::Pipeline",
                .layout = layout,
                .vertex = VertexState {
                    .attribute_layouts = {
                        { attribute_layout },
                    },
                },
                .primitive = PrimitiveState::get_default(),
                .depth_stencil = None(),
                .multi_sample = MultiSampleState::get_default(),
                .fragment = FragmentStage {
                    .targets = {
                        ColorTargetState {
                            .format = TextureFormat::RGBA16Float,
                            .write_mask = ColorWrite::All,
                        },
                    },
                },
            };

            m_pipeline = Renderer::device().create_render_pipeline(m_shader, m_shader, spec);
        }

        {
            auto shader = ShaderProcessor::process_file("Assets/Shaders/WGSL/OneShot/CubeTexConvolution.wgsl").unwrap();
            ShaderModuleSpec shader_spec {
                .label = "CubeTexGen::conv_shader",
                .type = WGSLShader {
                    .source = shader,
                },
                .vertex_entry_point = "vs_main",
                .fragment_entry_point = "fs_main",
            };

            m_conv_shader = Renderer::device().create_shader_module(shader_spec);

            std::vector bgl_entries {
                BindGroupLayoutEntry {
                    .binding = 0,
                    .visibility = ShaderStage::Vertex,
                    .type = BindingType::Buffer {
                        .type = BufferBindingType::Uniform {},
                    },
                    .count = 1,
                },
                BindGroupLayoutEntry {
                    .binding = 1,
                    .visibility = ShaderStage::Fragment,
                    .type = BindingType::Texture {
                        .sample_type = TextureSampleType::Float {},
                        .view_dimension = TextureViewDimension::Cube,
                        .multi_sampled = false,
                    },
                    .count = 1,
                },
                BindGroupLayoutEntry {
                    .binding = 2,
                    .visibility = ShaderStage::Fragment,
                    .type = BindingType::Sampler {
                        .type = SamplerBindingType::Filtering,
                    },
                    .count = 1,
                },
            };
            BindGroupLayoutSpec bgl_spec {
                .label = "CubeTexGen::conv_bgl",
                .entries = bgl_entries
            };
            m_conv_bind_group_layout = Renderer::device().create_bind_group_layout(bgl_spec);

            std::array layouts = {
                m_conv_bind_group_layout
            };

            PipelineLayoutSpec layout_spec {
                .label = "CubeTexGen::PipelineLayout",
                .bind_group_layouts = layouts,
            };
            auto layout = Renderer::device().create_pipeline_layout(layout_spec);

            std::array attributes {
                VertexAttribute {
                    .type = ElementType::Float3,
                    .shader_location = 0,
                },
            };
            auto attribute_layout = VertexBufferLayout::create(attributes);

            auto depth = DepthStencilState::get_default();
            depth.depth_write_enabled = false;
            depth.depth_compare = CompareFunction::Always;

            RenderPipelineSpec spec {
                .label = "CubeTexGen::conv_pipeline",
                .layout = layout,
                .vertex = VertexState {
                    .attribute_layouts = {
                        { attribute_layout },
                    },
                },
                .primitive = PrimitiveState::get_default(),
                .depth_stencil = None(),
                .multi_sample = MultiSampleState::get_default(),
                .fragment = FragmentStage {
                    .targets = {
                        ColorTargetState {
                            .format = TextureFormat::RGBA16Float,
                            .write_mask = ColorWrite::All,
                        },
                    },
                },
            };

            m_conv_pipeline = Renderer::device().create_render_pipeline(m_conv_shader, m_conv_shader, spec);
        }

        m_sampler = Renderer::device().create_sampler({
            .label = "sampler",
            .address_mode_u = AddressMode::ClampToEdge,
            .address_mode_v = AddressMode::ClampToEdge,
            .address_mode_w = AddressMode::ClampToEdge,
        });

        m_cube_vertices = Renderer::device().create_buffer({
            .label = "Cube Verts",
            .usage = BufferUsage::Vertex | BufferUsage::CopyDst,
            .size = 36 * sizeof(Vector3),
            .mapped = false,
        });

        Renderer::device().write_buffer<Vector3>(m_cube_vertices, 0, CUBE_VERTICES);

        auto perspective = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        m_capture_views = {
            perspective * lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { -1.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, 1.0f, 0.0f }),
            perspective * lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 1.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, 1.0f, 0.0f }),
            perspective * lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, 1.0f, 0.0f }, glm::vec3 { 0.0f, 0.0f, -1.0f }),
            perspective * lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, -1.0f, 0.0f }, glm::vec3 { 0.0f, 0.0f, 1.0f }),
            perspective * lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, 0.0f, 1.0f }, glm::vec3 { 0.0f, 1.0f, 0.0f }),
            perspective * lookAt(glm::vec3 { 0.0f, 0.0f, 0.0f }, glm::vec3 { 0.0f, 0.0f, -1.0f }, glm::vec3 { 0.0f, 1.0f, 0.0f }),
        };

        for (u32 i = 0; i < 6; ++i) {
            TextureSpec rt_spec {
                .label = "CubeTexGen::CrapGPU",
                .usage = TextureUsage::RenderAttachment | TextureUsage::CopySrc,
                .dimension = TextureDimension::D2,
                .size = { 512, 512, 1 },
                .format = TextureFormat::RGBA16Float,
                .sample_count = 1,
                .aspect = TextureAspect::All,
                .generate_mip_maps = false,
            };

            m_render_textures[i] = Renderer::device().create_texture(rt_spec);

            m_view_data[i] = UniformBuffer<ViewData>::create(Renderer::device());
            m_view_data[i].data.view = m_capture_views[i];
            m_view_data[i].flush();
        }
    }

    auto IBLIrradiance::generate(GPU::Texture const& input_texture) -> GPU::Texture
    {
        ZoneScoped;
        GPU::Utils::RenderDoc::start_capture();
        auto encoder = Renderer::device().create_command_encoder();

        encoder.push_debug_group("Cubemap");
        auto texture = generate_cubemap(encoder, input_texture);
        encoder.pop_debug_group();
        encoder.push_debug_group("Convolution");
        auto convoluted_texture = generate_convoluted_cubemap(encoder, texture);
        encoder.pop_debug_group();

        Renderer::device().submit_command_buffer(encoder.finish());
        encoder.release();
        GPU::Utils::RenderDoc::end_capture();
        return convoluted_texture;
    }

    auto IBLIrradiance::generate_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& input_texture) -> GPU::Texture
    {
        ZoneScoped;
        using namespace GPU;

        TextureSpec texture_spec {
            .label = "CubeTexGen::cube_texture",
            .usage = TextureUsage::TextureBinding | TextureUsage::CopyDst,
            .dimension = TextureDimension::D2,
            .size = { 512, 512, 6 },
            .format = TextureFormat::RGBA16Float,
            .sample_count = 1,
            .aspect = TextureAspect::All,
            .generate_mip_maps = false,
            .initialize_view = false,
        };

        auto texture = Renderer::device().create_texture(texture_spec);
        texture.view = texture.create_view({
            .label = "View"sv,
            .usage = texture_spec.usage,
            .dimension = TextureViewDimension::Cube, // TODO: Make configurable
            .format = texture_spec.format,
            .base_mip_level = 0, // TODO: Make configurable
            .mip_level_count = 1,
            .base_array_layer = 0,        // TODO: Make configurable
            .array_layer_count = 6,       // TODO: Make configurable
            .aspect = texture_spec.aspect // TODO: Make configurable
        });

        for (u32 i = 0; i < 6; ++i) {
            std::vector entries {
                BindGroupEntry {
                    .binding = 0,
                    .resource = BufferBinding {
                        .buffer = m_view_data[i].buffer(),
                        .offset = 0,
                        .size = m_view_data[i].size(),
                    },
                },
                BindGroupEntry {
                    .binding = 1,
                    .resource = input_texture.view,
                },
                BindGroupEntry {
                    .binding = 2,
                    .resource = m_sampler,
                }
            };

            m_bind_group = Renderer::device().create_bind_group(m_bind_group_layout,
                {
                    .label = "CubeTexGen::bind_group",
                    .entries = entries,
                });

            std::array attachments {
                RenderPassColorAttachment {
                    .view = m_render_textures[i].view,
                    .load_op = LoadOp::Clear,
                    .store_op = StoreOp::Store,
                    .clear_color = Color::Black,
                },
            };
            RenderPassSpec spec {
                .label = "CubeTexGen::render_pass",
                .color_attachments = attachments,
            };
            auto pass = encoder.begin_rendering(spec);

            pass.set_pipeline(m_pipeline);
            pass.set_bind_group(m_bind_group, 0);
            pass.set_vertex_buffer(0, m_cube_vertices);

            pass.draw({ 0, 36 }, { 0, 1 });

            pass.end();
            pass.release();
        }

        for (u32 i = 0; i < 6; ++i) {
            encoder.copy_texture_to_texture(m_render_textures[i], texture, { 512, 512 }, 0, 0, 0, i);
        }

        return texture;
    }

    auto IBLIrradiance::generate_convoluted_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& input_texture) -> GPU::Texture
    {
        ZoneScoped;
        using namespace GPU;

        TextureSpec texture_spec {
            .label = "CubeTexGen::conv_texture",
            .usage = TextureUsage::TextureBinding | TextureUsage::CopyDst,
            .dimension = TextureDimension::D2,
            .size = { 512, 512, 6 },
            .format = TextureFormat::RGBA16Float,
            .sample_count = 1,
            .aspect = TextureAspect::All,
            .generate_mip_maps = false,
            .initialize_view = false,
        };

        auto texture = Renderer::device().create_texture(texture_spec);
        texture.view = texture.create_view({
            .label = "View"sv,
            .usage = texture_spec.usage,
            .dimension = TextureViewDimension::Cube, // TODO: Make configurable
            .format = texture_spec.format,
            .base_mip_level = 0, // TODO: Make configurable
            .mip_level_count = 1,
            .base_array_layer = 0,        // TODO: Make configurable
            .array_layer_count = 6,       // TODO: Make configurable
            .aspect = texture_spec.aspect // TODO: Make configurable
        });

        for (u32 i = 0; i < 6; ++i) {
            std::vector entries {
                BindGroupEntry {
                    .binding = 0,
                    .resource = BufferBinding {
                        .buffer = m_view_data[i].buffer(),
                        .offset = 0,
                        .size = m_view_data[i].size(),
                    },
                },
                BindGroupEntry {
                    .binding = 1,
                    .resource = input_texture.view,
                },
                BindGroupEntry {
                    .binding = 2,
                    .resource = m_sampler,
                }
            };

            m_conv_bind_group = Renderer::device().create_bind_group(m_conv_bind_group_layout,
                {
                    .label = "CubeTexGen::bind_group",
                    .entries = entries,
                });

            std::array attachments {
                RenderPassColorAttachment {
                    .view = m_render_textures[i].view,
                    .load_op = LoadOp::Clear,
                    .store_op = StoreOp::Store,
                    .clear_color = Color::Black,
                },
            };
            RenderPassSpec spec {
                .label = "CubeTexGen::render_pass",
                .color_attachments = attachments,
            };
            auto pass = encoder.begin_rendering(spec);

            pass.set_pipeline(m_conv_pipeline);
            pass.set_bind_group(m_conv_bind_group, 0);
            pass.set_vertex_buffer(0, m_cube_vertices);

            pass.draw({ 0, 36 }, { 0, 1 });

            pass.end();
            pass.release();
        }

        for (u32 i = 0; i < 6; ++i) {
            encoder.copy_texture_to_texture(m_render_textures[i], texture, { 512, 512 }, 0, 0, 0, i);
        }

        return texture;
    }
}
