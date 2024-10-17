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
        m_BindGroup.Release();
    }

    constexpr auto EQUIRECT_TO_CUBE_MAP_PATH = "Assets/Shaders/Slang/EquirectToCubeMap.slang";
    constexpr auto CUBEMAP_CONVOLUTION_PATH = "Assets/Shaders/Slang/CubeMapConvolution.slang";

    void IrradianceIBLGenerator::Initialize()
    {
        ZoneScoped;
        using namespace GPU;
        {
            auto compiled = ShaderProcessor::CompileSlang(EQUIRECT_TO_CUBE_MAP_PATH).Unwrap();
            compiled.Metadata.UseDepth = false;

            m_CubeMapGeneratorShader = MakeRef<ShaderAsset>(compiled, std::vector { TextureFormat::RGBA16Float });
        }

        {
            auto compiled = ShaderProcessor::CompileSlang(CUBEMAP_CONVOLUTION_PATH).Unwrap();
            compiled.Metadata.UseDepth = false;

            m_CubeMapConvolutionShader = MakeRef<ShaderAsset>(compiled, std::vector { TextureFormat::RGBA16Float });
        }

        m_Sampler = Renderer::Device().CreateSampler({
            .label = "sampler"sv,
            .AddressModeU = AddressMode::ClampToEdge,
            .AddressModeV = AddressMode::ClampToEdge,
            .AddressModeW = AddressMode::ClampToEdge,
        });

        m_CubeVertexBuffer = Renderer::Device().CreateBuffer({
            .Label = "Cube Verts"sv,
            .Usage = BufferUsage::Vertex | BufferUsage::CopyDst,
            .Size = 36 * sizeof(Vector3),
            .Mapped = false,
        });

        Renderer::Device().WriteBuffer<Vector3>(m_CubeVertexBuffer, 0, CUBE_VERTICES);

        auto perspective = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        m_CaptureViews = {
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

            m_RenderTextures[i] = Renderer::Device().CreateTexture(rt_spec);

            m_PerFaceViewData[i] = UniformBuffer<ViewData>::Create(Renderer::Device());
            m_PerFaceViewData[i].Data.View = m_CaptureViews[i];
            m_PerFaceViewData[i].Flush();
        }
    }

    auto IrradianceIBLGenerator::Generate(GPU::Texture const& inputTexture) -> GPU::Texture
    {
        ZoneScoped;
        // GPU::Utils::RenderDoc::StartCapture();
        auto encoder = Renderer::Device().CreateCommandEncoder();

        encoder.PushDebugGroup("Cubemap");
        auto texture = GenerateCubemap(encoder, inputTexture);
        encoder.PopDebugGroup();

        encoder.PushDebugGroup("Convolution");
        auto convoluted_texture = GenerateConvolutedCubemap(encoder, texture);
        encoder.PopDebugGroup();

        Renderer::Device().SubmitCommandBuffer(encoder.Finish());
        encoder.Release();
        // GPU::Utils::RenderDoc::EndCapture();

        m_BindGroup.Release();
        m_ConvBindGroup.Release();
        return convoluted_texture;
    }

    auto IrradianceIBLGenerator::GenerateCubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture
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

        auto texture = Renderer::Device().CreateTexture(texture_spec);
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
                        .TargetBuffer = m_PerFaceViewData[i].Buffer(),
                        .Offset = 0,
                        .Size = m_PerFaceViewData[i].Size(),
                    },
                },
                BindGroupEntry {
                    .Binding = 1,
                    .Resource = inputTexture.View,
                },
                BindGroupEntry {
                    .Binding = 2,
                    .Resource = m_Sampler,
                }
            };

            m_BindGroup = Renderer::Device().CreateBindGroup(m_CubeMapGeneratorShader->GetBindGroupLayout(0).Unwrap(),
                {
                    .Label = "CubeTexGen::bind_group"sv,
                    .Entries = entries,
                });

            std::array attachments {
                RenderPassColorAttachment {
                    .View = m_RenderTextures[i].View,
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

            pass.SetPipeline(m_CubeMapGeneratorShader->Pipeline());
            pass.SetBindGroup(m_BindGroup, 0);
            pass.SetVertexBuffer(0, m_CubeVertexBuffer);

            pass.Draw({ 0, 36 }, { 0, 1 });

            pass.End();
            pass.Release();
        }

        for (u32 i = 0; i < 6; ++i) {
            encoder.CopyTextureToTexture(m_RenderTextures[i], texture, { 512, 512 }, 0, 0, 0, i);
        }

        return texture;
    }

    auto IrradianceIBLGenerator::GenerateConvolutedCubemap(GPU::CommandEncoder& encoder, GPU::Texture const& inputTexture) -> GPU::Texture
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

        auto texture = Renderer::Device().CreateTexture(texture_spec);
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
                        .TargetBuffer = m_PerFaceViewData[i].Buffer(),
                        .Offset = 0,
                        .Size = m_PerFaceViewData[i].Size(),
                    },
                },
                BindGroupEntry {
                    .Binding = 1,
                    .Resource = inputTexture.View,
                },
                BindGroupEntry {
                    .Binding = 2,
                    .Resource = m_Sampler,
                }
            };

            m_ConvBindGroup = Renderer::Device().CreateBindGroup(m_CubeMapConvolutionShader->GetBindGroupLayout(0).Unwrap(),
                {
                    .Label = "CubeTexGen::bind_group"sv,
                    .Entries = entries,
                });

            std::array attachments {
                RenderPassColorAttachment {
                    .View = m_RenderTextures[i].View,
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

            pass.SetPipeline(m_CubeMapConvolutionShader->Pipeline());
            pass.SetBindGroup(m_ConvBindGroup, 0);
            pass.SetVertexBuffer(0, m_CubeVertexBuffer);

            pass.Draw({ 0, 36 }, { 0, 1 });

            pass.End();
            pass.Release();
        }

        for (u32 i = 0; i < 6; ++i) {
            encoder.CopyTextureToTexture(m_RenderTextures[i], texture, { 512, 512 }, 0, 0, 0, i);
        }

        return texture;
    }
}
