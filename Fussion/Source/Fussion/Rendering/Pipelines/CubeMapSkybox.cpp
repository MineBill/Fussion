#include "CubeMapSkybox.h"

#include "FussionPCH.h"
#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"
#include "tracy/Tracy.hpp"

namespace Fussion {

    CubeSkybox::~CubeSkybox()
    {
        ZoneScoped;
        m_shader.Release();
        m_pipeline.Release();
    }

    void CubeSkybox::init(std::vector<GPU::BindGroupLayout> layouts)
    {
        ZoneScoped;
        using namespace GPU;
        auto shader_src = ShaderProcessor::ProcessFile(
            "Assets/Shaders/WGSL/CubeSkybox.wgsl")
                              .Unwrap();

        ShaderModuleSpec shader_spec {
            .Label = "CubeSkybox::Shader",
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
                .Visibility = ShaderStage::Fragment,
                .Type = BindingType::Texture {
                    .SampleType = TextureSampleType::Float {},
                    .ViewDimension = TextureViewDimension::Cube,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            BindGroupLayoutEntry {
                .Binding = 1,
                .Visibility = ShaderStage::Fragment,
                .Type = BindingType::Sampler {
                    .Type = SamplerBindingType::Filtering,
                },
                .Count = 1,
            },
        };
        BindGroupLayoutSpec bgl_spec {
            .Label = "poop",
            .Entries = bgl_entries
        };
        m_bind_group_layout = Renderer::Device().CreateBindGroupLayout(bgl_spec);

        layouts.push_back(m_bind_group_layout);

        PipelineLayoutSpec layout_spec {
            .Label = "asdasd",
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
            .Label = "CubeSkybox::Pipeline",
            .Layout = layout,
            .Vertex = VertexState {
                .AttributeLayouts = {
                    { attribute_layout },
                },
            },
            .Primitive = {
                .Topology = PrimitiveTopology::TriangleList,
                .StripIndexFormat = None(),
                .FrontFace = FrontFace::Ccw,
                .Cull = Face::None,
            },
            .DepthStencil = depth,
            .MultiSample = MultiSampleState::Default(),
            .Fragment = FragmentStage {
                .Targets = {
                    ColorTargetState {
                        .Format = TextureFormat::RGBA16Float,
                        .Blend = BlendState::Default(),
                        .WriteMask = ColorWrite::All,
                    },
                },
            },
        };

        m_pipeline = Renderer::Device().CreateRenderPipeline(m_shader, m_shader, spec);

        m_sampler = Renderer::Device().CreateSampler({
            .label = "sampler",
        });

        std::vector<Vector3> vertices {
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

        m_cube_vertices = Renderer::Device().CreateBuffer({
            .Label = "Cube Verts",
            .Usage = BufferUsage::Vertex | BufferUsage::CopyDst,
            .Size = 36 * sizeof(Vector3),
            .Mapped = false,
        });

        Renderer::Device().WriteBuffer<Vector3>(m_cube_vertices, 0, vertices);
    }

    void CubeSkybox::execute(GPU::RenderPassEncoder& pass)
    {
        ZoneScoped;
        if (m_bind_group.Handle == nullptr) {
            return;
        }
        using namespace GPU;

        // std::vector color_attachments {
        //     RenderPassColorAttachment {
        //         .view = render_target,
        //         .load_op = LoadOp::Load,
        //         .store_op = StoreOp::Store,
        //         .clear_color = Color::White,
        //     },
        // };
        // RenderPassSpec spec {
        //     .label = "CubeSkybox::RenderPass",
        //     .color_attachments = {},
        // };
        //
        // auto pass = encoder.begin_rendering(spec);
        // defer({
        //     pass.end();
        //     pass.release();
        // });

        pass.SetPipeline(m_pipeline);
        pass.SetVertexBuffer(0, m_cube_vertices);
        pass.SetBindGroup(m_bind_group, 2);
        pass.Draw({ 0, 36 }, { 0, 1 });
    }

    void CubeSkybox::set_map(GPU::Texture const& map)
    {
        ZoneScoped;
        using namespace GPU;
        m_bind_group.Release();

        std::vector entries {
            BindGroupEntry {
                .Binding = 0,
                .Resource = map.View,
            },
            BindGroupEntry {
                .Binding = 1,
                .Resource = m_sampler,
            }
        };

        m_bind_group = Renderer::Device().CreateBindGroup(m_bind_group_layout,
            {
                .Label = "asdasd",
                .Entries = entries,
            });
    }
} // namespace Fussion
