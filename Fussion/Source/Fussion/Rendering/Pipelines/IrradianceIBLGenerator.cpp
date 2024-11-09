#include "FussionPCH.h"
#include "IrradianceIBLGenerator.h"

#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"
#include "tracy/Tracy.hpp"

namespace Fussion {
    static std::vector const CUBE_VERTICES {
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

    IrradianceIBLGenerator::~IrradianceIBLGenerator()
    {
        m_bind_group.release();
    }

    constexpr auto EQUIRECT_TO_CUBE_MAP_PATH = "Assets/Shaders/Slang/EquirectToCubeMap.slang";
    constexpr auto CUBEMAP_CONVOLUTION_PATH = "Assets/Shaders/Slang/CubeMapConvolution.slang";

    void IrradianceIBLGenerator::init()
    {
        ZoneScoped;
        using namespace GPU;
        {
            auto compiled = ShaderProcessor::compile_slang(EQUIRECT_TO_CUBE_MAP_PATH).unwrap();
            compiled.metadata.use_depth = false;

            m_cube_map_generator_shader = make_ref<ShaderAsset>(compiled, std::vector { TextureFormat::RGBA16Float });
        }

        {
            auto compiled = ShaderProcessor::compile_slang(CUBEMAP_CONVOLUTION_PATH).unwrap();
            compiled.metadata.use_depth = false;

            m_cube_map_convolution_shader = make_ref<ShaderAsset>(compiled, std::vector { TextureFormat::RGBA16Float });
        }

        m_sampler = Renderer::device().create_sampler({
            .label = "sampler"sv,
            .address_mode_u = AddressMode::ClampToEdge,
            .address_mode_v = AddressMode::ClampToEdge,
            .address_mode_w = AddressMode::ClampToEdge,
        });

        m_cube_vertex_buffer = Renderer::device().create_buffer({
            .label = "Cube Verts"sv,
            .usage = BufferUsage::Vertex | BufferUsage::CopyDst,
            .size = 36 * sizeof(Vector3),
            .mapped_at_creation = false,
        });

        Renderer::device().write_buffer<Vector3>(m_cube_vertex_buffer, 0, CUBE_VERTICES);

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
            constexpr f32 resolution = 512.f;

            TextureSpec rt_spec {
                .label = "CubeTexGen::CrapGPU"sv,
                .usage = TextureUsage::RenderAttachment | TextureUsage::CopySrc,
                .dimension = TextureDimension::D2,
                .size = { resolution, resolution, 1 },
                .format = TextureFormat::RGBA16Float,
                .sample_count = 1,
                .aspect = TextureAspect::All,
                .generate_mip_maps = false,
            };

            m_render_textures[i] = Renderer::device().create_texture(rt_spec);

            m_per_face_view_data[i] = UniformBuffer<ViewData>::create(Renderer::device());
            m_per_face_view_data[i].Data.view = m_capture_views[i];
            m_per_face_view_data[i].flush();
        }
    }

    auto IrradianceIBLGenerator::generate(GPU::Texture const& inputTexture) -> GPU::Texture
    {
        ZoneScoped;
        // GPU::Utils::RenderDoc::StartCapture();
        auto encoder = Renderer::device().create_command_encoder();

        encoder.push_debug_group("Cubemap");
        auto texture = generate_cubemap(encoder, inputTexture);
        encoder.pop_debug_group();

        encoder.push_debug_group("Convolution");
        auto convoluted_texture = generate_convoluted_cubemap(encoder, texture);
        encoder.pop_debug_group();

        Renderer::device().submit_command_buffer(encoder.finish());
        encoder.release();
        // GPU::Utils::RenderDoc::EndCapture();

        m_bind_group.release();
        m_conv_bind_group.release();
        return convoluted_texture;
    }

    auto IrradianceIBLGenerator::generate_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture
    {
        ZoneScoped;
        using namespace GPU;

        TextureSpec texture_spec {
            .label = "CubeTexGen::cube_texture"sv,
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
            .base_array_layer = 0,          // TODO: Make configurable
            .array_layer_count = 6,         // TODO: Make configurable
            .aspect = texture_spec.aspect // TODO: Make configurable
        });

        for (u32 i = 0; i < 6; ++i) {
            std::vector entries {
                BindGroupEntry {
                    .binding = 0,
                    .resource = BufferBinding {
                        .target_buffer = m_per_face_view_data[i].buffer(),
                        .offset = 0,
                        .size = m_per_face_view_data[i].size(),
                    },
                },
                BindGroupEntry {
                    .binding = 1,
                    .resource = inputTexture.view,
                },
                BindGroupEntry {
                    .binding = 2,
                    .resource = m_sampler,
                }
            };

            m_bind_group = Renderer::device().create_bind_group(m_cube_map_generator_shader->get_bind_group_layout_for(0).unwrap(),
                {
                    .label = "CubeTexGen::bind_group"sv,
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
                .label = "CubeTexGen::render_pass"sv,
                .color_attachments = attachments,
            };
            auto pass = encoder.begin_rendering(spec);

            pass.set_pipeline(m_cube_map_generator_shader->pipeline());
            pass.set_bind_group(m_bind_group, 0);
            pass.set_vertex_buffer(0, m_cube_vertex_buffer);

            pass.draw({ 0, 36 }, { 0, 1 });

            pass.end();
            pass.release();
        }

        for (u32 i = 0; i < 6; ++i) {
            encoder.copy_texture_to_texture(m_render_textures[i], texture, { 512, 512 }, 0, 0, 0, i);
        }

        return texture;
    }

    auto IrradianceIBLGenerator::generate_convoluted_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture
    {
        ZoneScoped;
        using namespace GPU;

        TextureSpec texture_spec {
            .label = "CubeTexGen::conv_texture"sv,
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
            .base_array_layer = 0,          // TODO: Make configurable
            .array_layer_count = 6,         // TODO: Make configurable
            .aspect = texture_spec.aspect // TODO: Make configurable
        });

        for (u32 i = 0; i < 6; ++i) {
            std::vector entries {
                BindGroupEntry {
                    .binding = 0,
                    .resource = BufferBinding {
                        .target_buffer = m_per_face_view_data[i].buffer(),
                        .offset = 0,
                        .size = m_per_face_view_data[i].size(),
                    },
                },
                BindGroupEntry {
                    .binding = 1,
                    .resource = inputTexture.view,
                },
                BindGroupEntry {
                    .binding = 2,
                    .resource = m_sampler,
                }
            };

            m_conv_bind_group = Renderer::device().create_bind_group(m_cube_map_convolution_shader->get_bind_group_layout_for(0).unwrap(),
                {
                    .label = "CubeTexGen::bind_group"sv,
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
                .label = "CubeTexGen::render_pass"sv,
                .color_attachments = attachments,
            };
            auto pass = encoder.begin_rendering(spec);

            pass.set_pipeline(m_cube_map_convolution_shader->pipeline());
            pass.set_bind_group(m_conv_bind_group, 0);
            pass.set_vertex_buffer(0, m_cube_vertex_buffer);

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
