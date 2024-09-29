#include "CubeMapSkybox.h"

#include "FussionPCH.h"
#include "GPU/ShaderProcessor.h"
#include "Rendering/Renderer.h"
#include "tracy/Tracy.hpp"

namespace Fussion {

    CubeSkybox::~CubeSkybox()
    {
        ZoneScoped;
        m_shader.release();
        m_pipeline.release();
    }

    void CubeSkybox::init(std::vector<GPU::BindGroupLayout> layouts)
    {
        ZoneScoped;
        using namespace GPU;
        auto shader_src = ShaderProcessor::process_file(
            "Assets/Shaders/WGSL/CubeSkybox.wgsl")
                              .unwrap();

        ShaderModuleSpec shader_spec {
            .label = "CubeSkybox::Shader",
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
                .visibility = ShaderStage::Fragment,
                .type = BindingType::Texture {
                    .sample_type = TextureSampleType::Float {},
                    .view_dimension = TextureViewDimension::Cube,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            BindGroupLayoutEntry {
                .binding = 1,
                .visibility = ShaderStage::Fragment,
                .type = BindingType::Sampler {
                    .type = SamplerBindingType::Filtering,
                },
                .count = 1,
            },
        };
        BindGroupLayoutSpec bgl_spec {
            .label = "poop",
            .entries = bgl_entries
        };
        m_bind_group_layout = Renderer::device().create_bind_group_layout(bgl_spec);

        layouts.push_back(m_bind_group_layout);

        PipelineLayoutSpec layout_spec {
            .label = "asdasd",
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
            .label = "CubeSkybox::Pipeline",
            .layout = layout,
            .vertex = VertexState {
                .attribute_layouts = {
                    { attribute_layout },
                },
            },
            .primitive = {
                .topology = PrimitiveTopology::TriangleList,
                .strip_index_format = None(),
                .front_face = FrontFace::Ccw,
                .cull = Face::None,
            },
            .depth_stencil = depth,
            .multi_sample = MultiSampleState::get_default(),
            .fragment = FragmentStage {
                .targets = {
                    ColorTargetState {
                        .format = TextureFormat::RGBA16Float,
                        .blend = BlendState::get_default(),
                        .write_mask = ColorWrite::All,
                    },
                },
            },
        };

        m_pipeline = Renderer::device().create_render_pipeline(m_shader, m_shader, spec);

        m_sampler = Renderer::device().create_sampler({
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

        m_cube_vertices = Renderer::device().create_buffer({
            .label = "Cube Verts",
            .usage = BufferUsage::Vertex | BufferUsage::CopyDst,
            .size = 36 * sizeof(Vector3),
            .mapped = false,
        });

        Renderer::device().write_buffer<Vector3>(m_cube_vertices, 0, vertices);
    }

    void CubeSkybox::execute(GPU::RenderPassEncoder& pass)
    {
        ZoneScoped;
        if (m_bind_group.handle == nullptr) {
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

        pass.set_pipeline(m_pipeline);
        pass.set_vertex_buffer(0, m_cube_vertices);
        pass.set_bind_group(m_bind_group, 2);
        pass.draw({ 0, 36 }, { 0, 1 });
    }

    void CubeSkybox::set_map(GPU::Texture const& map)
    {
        ZoneScoped;
        using namespace GPU;
        m_bind_group.release();

        std::vector entries {
            BindGroupEntry {
                .binding = 0,
                .resource = map.view,
            },
            BindGroupEntry {
                .binding = 1,
                .resource = m_sampler,
            }
        };

        m_bind_group = Renderer::device().create_bind_group(m_bind_group_layout,
            {
                .label = "asdasd",
                .entries = entries,
            });
    }
} // namespace Fussion
