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
        m_bind_group.Release();
    }

    void IBLIrradiance::init()
    {
        ZoneScoped;
        using namespace GPU;
        {
            auto shader_src = ShaderProcessor::ProcessFile(
                "Assets/Shaders/WGSL/OneShot/CubeTexGen.wgsl")
                                  .Unwrap();

            ShaderModuleSpec shader_spec {
                .Label = "CubeTexGen::Shader",
                .Type = WGSLShader {
                    .Source = shader_src,
                },
                .VertexEntryPoint = "vs_main",
                .FragmentEntryPoint = "fs_main",
            };

            m_shader = Renderer::Device().CreateShaderModule(shader_spec);

            std::vector bgl_entries {
                BindGroupLayoutEntry {
                    .Binding = 0,
                    .Visibility = ShaderStage::Vertex,
                    .Type = BindingType::Buffer {
                        .Type = BufferBindingType::Uniform {},
                    },
                    .Count = 1,
                },
                BindGroupLayoutEntry {
                    .Binding = 1,
                    .Visibility = ShaderStage::Fragment,
                    .Type = BindingType::Texture {
                        .SampleType = TextureSampleType::Float {},
                        .ViewDimension = TextureViewDimension::D2,
                        .MultiSampled = false,
                    },
                    .Count = 1,
                },
                BindGroupLayoutEntry {
                    .Binding = 2,
                    .Visibility = ShaderStage::Fragment,
                    .Type = BindingType::Sampler {
                        .Type = SamplerBindingType::Filtering,
                    },
                    .Count = 1,
                },

            };
            BindGroupLayoutSpec bgl_spec {
                .Label = "CubeTexGen::BGL",
                .Entries = bgl_entries
            };
            m_bind_group_layout = Renderer::Device().CreateBindGroupLayout(bgl_spec);

            std::array layouts = {
                m_bind_group_layout
            };

            PipelineLayoutSpec layout_spec {
                .Label = "CubeTexGen::PipelineLayout",
                .BindGroupLayouts = layouts,
            };
            auto layout = Renderer::Device().CreatePipelineLayout(layout_spec);

            std::array attributes {
                VertexAttribute {
                    .Type = ElementType::Float3,
                    .ShaderLocation = 0,
                },
            };
            auto attribute_layout = VertexBufferLayout::Create(attributes);

            auto depth = DepthStencilState::Default();
            depth.DepthWriteEnabled = false;
            depth.DepthCompare = CompareFunction::Always;

            RenderPipelineSpec spec {
                .Label = "CubeTexGen::Pipeline",
                .Layout = layout,
                .Vertex = VertexState {
                    .AttributeLayouts = {
                        { attribute_layout },
                    },
                },
                .Primitive = PrimitiveState::Default(),
                .DepthStencil = None(),
                .MultiSample = MultiSampleState::Default(),
                .Fragment = FragmentStage {
                    .Targets = {
                        ColorTargetState {
                            .Format = TextureFormat::RGBA16Float,
                            .WriteMask = ColorWrite::All,
                        },
                    },
                },
            };

            m_pipeline = Renderer::Device().CreateRenderPipeline(m_shader, m_shader, spec);
        }

        {
            auto shader = ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/OneShot/CubeTexConvolution.wgsl").Unwrap();
            ShaderModuleSpec shader_spec {
                .Label = "CubeTexGen::conv_shader",
                .Type = WGSLShader {
                    .Source = shader,
                },
                .VertexEntryPoint = "vs_main",
                .FragmentEntryPoint = "fs_main",
            };

            m_conv_shader = Renderer::Device().CreateShaderModule(shader_spec);

            std::vector bgl_entries {
                BindGroupLayoutEntry {
                    .Binding = 0,
                    .Visibility = ShaderStage::Vertex,
                    .Type = BindingType::Buffer {
                        .Type = BufferBindingType::Uniform {},
                    },
                    .Count = 1,
                },
                BindGroupLayoutEntry {
                    .Binding = 1,
                    .Visibility = ShaderStage::Fragment,
                    .Type = BindingType::Texture {
                        .SampleType = TextureSampleType::Float {},
                        .ViewDimension = TextureViewDimension::Cube,
                        .MultiSampled = false,
                    },
                    .Count = 1,
                },
                BindGroupLayoutEntry {
                    .Binding = 2,
                    .Visibility = ShaderStage::Fragment,
                    .Type = BindingType::Sampler {
                        .Type = SamplerBindingType::Filtering,
                    },
                    .Count = 1,
                },
            };
            BindGroupLayoutSpec bgl_spec {
                .Label = "CubeTexGen::conv_bgl",
                .Entries = bgl_entries
            };
            m_conv_bind_group_layout = Renderer::Device().CreateBindGroupLayout(bgl_spec);

            std::array layouts = {
                m_conv_bind_group_layout
            };

            PipelineLayoutSpec layout_spec {
                .Label = "CubeTexGen::PipelineLayout",
                .BindGroupLayouts = layouts,
            };
            auto layout = Renderer::Device().CreatePipelineLayout(layout_spec);

            std::array attributes {
                VertexAttribute {
                    .Type = ElementType::Float3,
                    .ShaderLocation = 0,
                },
            };
            auto attribute_layout = VertexBufferLayout::Create(attributes);

            auto depth = DepthStencilState::Default();
            depth.DepthWriteEnabled = false;
            depth.DepthCompare = CompareFunction::Always;

            RenderPipelineSpec spec {
                .Label = "CubeTexGen::conv_pipeline",
                .Layout = layout,
                .Vertex = VertexState {
                    .AttributeLayouts = {
                        { attribute_layout },
                    },
                },
                .Primitive = PrimitiveState::Default(),
                .DepthStencil = None(),
                .MultiSample = MultiSampleState::Default(),
                .Fragment = FragmentStage {
                    .Targets = {
                        ColorTargetState {
                            .Format = TextureFormat::RGBA16Float,
                            .WriteMask = ColorWrite::All,
                        },
                    },
                },
            };

            m_conv_pipeline = Renderer::Device().CreateRenderPipeline(m_conv_shader, m_conv_shader, spec);
        }

        m_sampler = Renderer::Device().CreateSampler({
            .label = "sampler",
            .AddressModeU = AddressMode::ClampToEdge,
            .AddressModeV = AddressMode::ClampToEdge,
            .AddressModeW = AddressMode::ClampToEdge,
        });

        m_cube_vertices = Renderer::Device().CreateBuffer({
            .Label = "Cube Verts",
            .Usage = BufferUsage::Vertex | BufferUsage::CopyDst,
            .Size = 36 * sizeof(Vector3),
            .Mapped = false,
        });

        Renderer::Device().WriteBuffer<Vector3>(m_cube_vertices, 0, CUBE_VERTICES);

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
                .Label = "CubeTexGen::CrapGPU",
                .Usage = TextureUsage::RenderAttachment | TextureUsage::CopySrc,
                .Dimension = TextureDimension::D2,
                .Size = { 512, 512, 1 },
                .Format = TextureFormat::RGBA16Float,
                .SampleCount = 1,
                .Aspect = TextureAspect::All,
                .GenerateMipMaps = false,
            };

            m_render_textures[i] = Renderer::Device().CreateTexture(rt_spec);

            m_view_data[i] = UniformBuffer<ViewData>::Create(Renderer::Device());
            m_view_data[i].Data.view = m_capture_views[i];
            m_view_data[i].flush();
        }
    }

    auto IBLIrradiance::generate(GPU::Texture const& input_texture) -> GPU::Texture
    {
        ZoneScoped;
        GPU::Utils::RenderDoc::StartCapture();
        auto encoder = Renderer::Device().CreateCommandEncoder();

        encoder.PushDebugGroup("Cubemap");
        auto texture = generate_cubemap(encoder, input_texture);
        encoder.PopDebugGroup();
        encoder.PushDebugGroup("Convolution");
        auto convoluted_texture = generate_convoluted_cubemap(encoder, texture);
        encoder.PopDebugGroup();

        Renderer::Device().SubmitCommandBuffer(encoder.Finish());
        encoder.Release();
        GPU::Utils::RenderDoc::EndCapture();
        return convoluted_texture;
    }

    auto IBLIrradiance::generate_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& input_texture) -> GPU::Texture
    {
        ZoneScoped;
        using namespace GPU;

        TextureSpec texture_spec {
            .Label = "CubeTexGen::cube_texture",
            .Usage = TextureUsage::TextureBinding | TextureUsage::CopyDst,
            .Dimension = TextureDimension::D2,
            .Size = { 512, 512, 6 },
            .Format = TextureFormat::RGBA16Float,
            .SampleCount = 1,
            .Aspect = TextureAspect::All,
            .GenerateMipMaps = false,
            .InitializeView = false,
        };

        auto texture = Renderer::Device().CreateTexture(texture_spec);
        texture.View = texture.CreateView({
            .Label = "View"sv,
            .Usage = texture_spec.Usage,
            .Dimension = TextureViewDimension::Cube, // TODO: Make configurable
            .Format = texture_spec.Format,
            .BaseMipLevel = 0, // TODO: Make configurable
            .MipLevelCount = 1,
            .BaseArrayLayer = 0,        // TODO: Make configurable
            .ArrayLayerCount = 6,       // TODO: Make configurable
            .Aspect = texture_spec.Aspect // TODO: Make configurable
        });

        for (u32 i = 0; i < 6; ++i) {
            std::vector entries {
                BindGroupEntry {
                    .Binding = 0,
                    .Resource = BufferBinding {
                        .TargetBuffer = m_view_data[i].Buffer(),
                        .Offset = 0,
                        .Size = m_view_data[i].Size(),
                    },
                },
                BindGroupEntry {
                    .Binding = 1,
                    .Resource = input_texture.View,
                },
                BindGroupEntry {
                    .Binding = 2,
                    .Resource = m_sampler,
                }
            };

            m_bind_group = Renderer::Device().CreateBindGroup(m_bind_group_layout,
                {
                    .Label = "CubeTexGen::bind_group",
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
                .Label = "CubeTexGen::render_pass",
                .ColorAttachments = attachments,
            };
            auto pass = encoder.BeginRendering(spec);

            pass.SetPipeline(m_pipeline);
            pass.SetBindGroup(m_bind_group, 0);
            pass.SetVertexBuffer(0, m_cube_vertices);

            pass.Draw({ 0, 36 }, { 0, 1 });

            pass.End();
            pass.Release();
        }

        for (u32 i = 0; i < 6; ++i) {
            encoder.CopyTextureToTexture(m_render_textures[i], texture, { 512, 512 }, 0, 0, 0, i);
        }

        return texture;
    }

    auto IBLIrradiance::generate_convoluted_cubemap(GPU::CommandEncoder& encoder, GPU::Texture const& input_texture) -> GPU::Texture
    {
        ZoneScoped;
        using namespace GPU;

        TextureSpec texture_spec {
            .Label = "CubeTexGen::conv_texture",
            .Usage = TextureUsage::TextureBinding | TextureUsage::CopyDst,
            .Dimension = TextureDimension::D2,
            .Size = { 512, 512, 6 },
            .Format = TextureFormat::RGBA16Float,
            .SampleCount = 1,
            .Aspect = TextureAspect::All,
            .GenerateMipMaps = false,
            .InitializeView = false,
        };

        auto texture = Renderer::Device().CreateTexture(texture_spec);
        texture.View = texture.CreateView({
            .Label = "View"sv,
            .Usage = texture_spec.Usage,
            .Dimension = TextureViewDimension::Cube, // TODO: Make configurable
            .Format = texture_spec.Format,
            .BaseMipLevel = 0, // TODO: Make configurable
            .MipLevelCount = 1,
            .BaseArrayLayer = 0,        // TODO: Make configurable
            .ArrayLayerCount = 6,       // TODO: Make configurable
            .Aspect = texture_spec.Aspect // TODO: Make configurable
        });

        for (u32 i = 0; i < 6; ++i) {
            std::vector entries {
                BindGroupEntry {
                    .Binding = 0,
                    .Resource = BufferBinding {
                        .TargetBuffer = m_view_data[i].Buffer(),
                        .Offset = 0,
                        .Size = m_view_data[i].Size(),
                    },
                },
                BindGroupEntry {
                    .Binding = 1,
                    .Resource = input_texture.View,
                },
                BindGroupEntry {
                    .Binding = 2,
                    .Resource = m_sampler,
                }
            };

            m_conv_bind_group = Renderer::Device().CreateBindGroup(m_conv_bind_group_layout,
                {
                    .Label = "CubeTexGen::bind_group",
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
                .Label = "CubeTexGen::render_pass",
                .ColorAttachments = attachments,
            };
            auto pass = encoder.BeginRendering(spec);

            pass.SetPipeline(m_conv_pipeline);
            pass.SetBindGroup(m_conv_bind_group, 0);
            pass.SetVertexBuffer(0, m_cube_vertices);

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
