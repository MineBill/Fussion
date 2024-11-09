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
        m_bind_group.Release();
    }

    constexpr auto EQUIRECT_TO_CUBE_MAP_PATH = "Assets/Shaders/Slang/EquirectToCubeMap.slang";
    constexpr auto CUBEMAP_CONVOLUTION_PATH = "Assets/Shaders/Slang/CubeMapConvolution.slang";

    void IrradianceIBLGenerator::init()
    {
        ZoneScoped;
        using namespace GPU;
        {
            auto compiled = ShaderProcessor::CompileSlang(EQUIRECT_TO_CUBE_MAP_PATH).unwrap();
            compiled.Metadata.UseDepth = false;

            m_cube_map_generator_shader = make_ref<ShaderAsset>(compiled, std::vector { TextureFormat::RGBA16Float });
        }

        {
            auto compiled = ShaderProcessor::CompileSlang(CUBEMAP_CONVOLUTION_PATH).unwrap();
            compiled.Metadata.UseDepth = false;

            m_cube_map_convolution_shader = make_ref<ShaderAsset>(compiled, std::vector { TextureFormat::RGBA16Float });
        }

        m_sampler = Renderer::device().CreateSampler({
            .label = "sampler"sv,
            .AddressModeU = AddressMode::ClampToEdge,
            .AddressModeV = AddressMode::ClampToEdge,
            .AddressModeW = AddressMode::ClampToEdge,
        });

        m_cube_vertex_buffer = Renderer::device().CreateBuffer({
            .Label = "Cube Verts"sv,
            .Usage = BufferUsage::Vertex | BufferUsage::CopyDst,
            .Size = 36 * sizeof(Vector3),
            .Mapped = false,
        });

        Renderer::device().WriteBuffer<Vector3>(m_cube_vertex_buffer, 0, CUBE_VERTICES);

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
                .Label = "CubeTexGen::CrapGPU"sv,
                .Usage = TextureUsage::RenderAttachment | TextureUsage::CopySrc,
                .Dimension = TextureDimension::D2,
                .Size = { resolution, resolution, 1 },
                .Format = TextureFormat::RGBA16Float,
                .SampleCount = 1,
                .Aspect = TextureAspect::All,
                .GenerateMipMaps = false,
            };

            m_render_textures[i] = Renderer::device().CreateTexture(rt_spec);

            m_per_face_view_data[i] = UniformBuffer<ViewData>::create(Renderer::device());
            m_per_face_view_data[i].Data.view = m_capture_views[i];
            m_per_face_view_data[i].flush();
        }
    }

    auto IrradianceIBLGenerator::generate(GPU::Texture const& inputTexture) -> GPU::Texture
    {
        ZoneScoped;
        // GPU::Utils::RenderDoc::StartCapture();
        auto encoder = Renderer::device().CreateCommandEncoder();

        encoder.PushDebugGroup("Cubemap");
        auto texture = generate_cubemap(encoder, inputTexture);
        encoder.PopDebugGroup();

        encoder.PushDebugGroup("Convolution");
        auto convoluted_texture = generate_convoluted_cubemap(encoder, texture);
        encoder.PopDebugGroup();

        Renderer::device().SubmitCommandBuffer(encoder.Finish());
        encoder.Release();
        // GPU::Utils::RenderDoc::EndCapture();

        m_bind_group.Release();
        m_conv_bind_group.Release();
        return convoluted_texture;
    }

    auto IrradianceIBLGenerator::generate_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture
    {
        ZoneScoped;
        using namespace GPU;

        TextureSpec texture_spec {
            .Label = "CubeTexGen::cube_texture"sv,
            .Usage = TextureUsage::TextureBinding | TextureUsage::CopyDst,
            .Dimension = TextureDimension::D2,
            .Size = { 512, 512, 6 },
            .Format = TextureFormat::RGBA16Float,
            .SampleCount = 1,
            .Aspect = TextureAspect::All,
            .GenerateMipMaps = false,
            .InitializeView = false,
        };

        auto texture = Renderer::device().CreateTexture(texture_spec);
        texture.View = texture.CreateView({
            .Label = "View"sv,
            .Usage = texture_spec.Usage,
            .Dimension = TextureViewDimension::Cube, // TODO: Make configurable
            .Format = texture_spec.Format,
            .BaseMipLevel = 0, // TODO: Make configurable
            .MipLevelCount = 1,
            .BaseArrayLayer = 0,          // TODO: Make configurable
            .ArrayLayerCount = 6,         // TODO: Make configurable
            .Aspect = texture_spec.Aspect // TODO: Make configurable
        });

        for (u32 i = 0; i < 6; ++i) {
            std::vector entries {
                BindGroupEntry {
                    .Binding = 0,
                    .Resource = BufferBinding {
                        .TargetBuffer = m_per_face_view_data[i].buffer(),
                        .Offset = 0,
                        .Size = m_per_face_view_data[i].size(),
                    },
                },
                BindGroupEntry {
                    .Binding = 1,
                    .Resource = inputTexture.View,
                },
                BindGroupEntry {
                    .Binding = 2,
                    .Resource = m_sampler,
                }
            };

            m_bind_group = Renderer::device().CreateBindGroup(m_cube_map_generator_shader->get_bind_group_layout_for(0).unwrap(),
                {
                    .Label = "CubeTexGen::bind_group"sv,
                    .Entries = entries,
                });

            std::array attachments {
                RenderPassColorAttachment {
                    .View = m_render_textures[i].View,
                    .LoadOp = LoadOp::Clear,
                    .StoreOp = StoreOp::Store,
                    .ClearColor = Color::Black,
                },
            };
            RenderPassSpec spec {
                .Label = "CubeTexGen::render_pass"sv,
                .ColorAttachments = attachments,
            };
            auto pass = encoder.BeginRendering(spec);

            pass.SetPipeline(m_cube_map_generator_shader->pipeline());
            pass.SetBindGroup(m_bind_group, 0);
            pass.SetVertexBuffer(0, m_cube_vertex_buffer);

            pass.Draw({ 0, 36 }, { 0, 1 });

            pass.End();
            pass.Release();
        }

        for (u32 i = 0; i < 6; ++i) {
            encoder.CopyTextureToTexture(m_render_textures[i], texture, { 512, 512 }, 0, 0, 0, i);
        }

        return texture;
    }

    auto IrradianceIBLGenerator::generate_convoluted_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture
    {
        ZoneScoped;
        using namespace GPU;

        TextureSpec texture_spec {
            .Label = "CubeTexGen::conv_texture"sv,
            .Usage = TextureUsage::TextureBinding | TextureUsage::CopyDst,
            .Dimension = TextureDimension::D2,
            .Size = { 512, 512, 6 },
            .Format = TextureFormat::RGBA16Float,
            .SampleCount = 1,
            .Aspect = TextureAspect::All,
            .GenerateMipMaps = false,
            .InitializeView = false,
        };

        auto texture = Renderer::device().CreateTexture(texture_spec);
        texture.View = texture.CreateView({
            .Label = "View"sv,
            .Usage = texture_spec.Usage,
            .Dimension = TextureViewDimension::Cube, // TODO: Make configurable
            .Format = texture_spec.Format,
            .BaseMipLevel = 0, // TODO: Make configurable
            .MipLevelCount = 1,
            .BaseArrayLayer = 0,          // TODO: Make configurable
            .ArrayLayerCount = 6,         // TODO: Make configurable
            .Aspect = texture_spec.Aspect // TODO: Make configurable
        });

        for (u32 i = 0; i < 6; ++i) {
            std::vector entries {
                BindGroupEntry {
                    .Binding = 0,
                    .Resource = BufferBinding {
                        .TargetBuffer = m_per_face_view_data[i].buffer(),
                        .Offset = 0,
                        .Size = m_per_face_view_data[i].size(),
                    },
                },
                BindGroupEntry {
                    .Binding = 1,
                    .Resource = inputTexture.View,
                },
                BindGroupEntry {
                    .Binding = 2,
                    .Resource = m_sampler,
                }
            };

            m_conv_bind_group = Renderer::device().CreateBindGroup(m_cube_map_convolution_shader->get_bind_group_layout_for(0).unwrap(),
                {
                    .Label = "CubeTexGen::bind_group"sv,
                    .Entries = entries,
                });

            std::array attachments {
                RenderPassColorAttachment {
                    .View = m_render_textures[i].View,
                    .LoadOp = LoadOp::Clear,
                    .StoreOp = StoreOp::Store,
                    .ClearColor = Color::Black,
                },
            };
            RenderPassSpec spec {
                .Label = "CubeTexGen::render_pass"sv,
                .ColorAttachments = attachments,
            };
            auto pass = encoder.BeginRendering(spec);

            pass.SetPipeline(m_cube_map_convolution_shader->pipeline());
            pass.SetBindGroup(m_conv_bind_group, 0);
            pass.SetVertexBuffer(0, m_cube_vertex_buffer);

            pass.Draw({ 0, 36 }, { 0, 1 });

            pass.End();
            pass.Release();
        }

        for (u32 i = 0; i < 6; ++i) {
            encoder.CopyTextureToTexture(m_render_textures[i], texture, { 512, 512 }, 0, 0, 0, i);
        }

        return texture;
    }
}
