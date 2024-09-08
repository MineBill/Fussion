#include "EditorPCH.h"

#include "SceneRenderer.h"
#include "Project/Project.h"

#include <Fussion/Core/Application.h>
#include <Fussion/GPU/ShaderProcessor.h>
#include <Fussion/Rendering/Renderer.h>
#include <Fussion/Core/Time.h>
#include <Fussion/Debug/Debug.h>
#include <Fussion/OS/FileSystem.h>
#include <tracy/Tracy.hpp>

#undef far
#undef near
#undef min
#undef max

using namespace Fussion;

void SceneRenderer::init()
{
    auto window_size = Application::inst()->window().size();

    create_scene_render_target(window_size);

    scene_view_data = UniformBuffer<ViewData>::create(Renderer::device(), std::string_view{ "View Data" });
    scene_light_data = UniformBuffer<LightData>::create(Renderer::device(), std::string_view{ "Light Data" });

    ///////////////////////
    /// BIND GROUP CREATION
    ///////////////////////

    setup_shadow_pass_render_target();

    {
        std::array entries{
            GPU::BindGroupLayoutEntry{
                .binding = 0,
                .visibility = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Buffer{
                    .type = GPU::BufferBindingType::Uniform{},
                    .has_dynamic_offset = false,
                    .min_binding_size = None(),
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 1,
                .visibility = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Buffer{
                    .type = GPU::BufferBindingType::Uniform{},
                    .has_dynamic_offset = false,
                    .min_binding_size = None(),
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 2,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture{
                    .sample_type = GPU::TextureSampleType::Depth{},
                    .view_dimension = GPU::TextureViewDimension::D2_Array,
                    .multi_sampled = false,
                },
                .count = 1,
            },
        };

        GPU::BindGroupLayoutSpec spec{
            .label = "Global BGL"sv,
            .entries = entries,
        };

        m_global_bind_group_layout = Renderer::device().create_bind_group_layout(spec);

        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .binding = 0,
                .resource = GPU::BufferBinding{
                    .buffer = scene_view_data.buffer(),
                    .offset = 0,
                    .size = scene_view_data.size(),
                }
            },
            GPU::BindGroupEntry{
                .binding = 1,
                .resource = GPU::BufferBinding{
                    .buffer = scene_light_data.buffer(),
                    .offset = 0,
                    .size = scene_light_data.size(),
                }
            },
            GPU::BindGroupEntry{
                .binding = 2,
                .resource = m_shadow_pass_render_target.view
            }
        };

        GPU::BindGroupSpec global_bg_spec{
            .label = "Global Bind Group"sv,
            .entries = bind_group_entries
        };

        m_global_bind_group = Renderer::device().create_bind_group(m_global_bind_group_layout, global_bg_spec);
    }

    // {
    //     std::array entries{
    //         GPU::BindGroupLayoutEntry{
    //             .Binding = 0,
    //             .Visibility = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment,
    //             .Type = GPU::BindingType::Buffer{
    //                 .Type = GPU::BufferBindingType::Uniform{},
    //                 .HasDynamicOffset = false,
    //                 .MinBindingSize = None(),
    //             },
    //             .Count = 1,
    //         },
    //     };
    //
    //     GPU::BindGroupLayoutSpec spec{
    //         .Label = "Scene BGL",
    //         .Entries = entries,
    //     };
    //
    //     m_SceneBindGroupLayout = Renderer::Device().CreateBindGroupLayout(spec);
    //
    //     std::array bind_group_entries{
    //         GPU::BindGroupEntry{
    //             .Binding = 0,
    //             .Resource = GPU::BufferBinding{
    //                 .Buffer = SceneViewData.GetBuffer(),
    //                 .Offset = 0,
    //                 .Size = SceneViewData.Size(),
    //             }
    //         }
    //     };
    //
    //     GPU::BindGroupSpec global_bg_spec{
    //         .Label = "Global Bind Group",
    //         .Entries = bind_group_entries
    //     };
    //
    //     m_SceneBindGroup = Renderer::Device().CreateBindGroup(m_SceneBindGroupLayout, global_bg_spec);
    // }

    {
        std::array entries{
            GPU::BindGroupLayoutEntry{
                .binding = 0,
                .visibility = GPU::ShaderStage::Vertex,
                .type = GPU::BindingType::Buffer{
                    .type = GPU::BufferBindingType::Storage{
                        .read_only = true,
                    },
                    .has_dynamic_offset = false,
                    .min_binding_size = None(),
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 1,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Buffer{
                    .type = GPU::BufferBindingType::Uniform{},
                    .has_dynamic_offset = false,
                    .min_binding_size = None(),
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 2,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture{
                    .sample_type = GPU::TextureSampleType::Float{},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 3,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture{
                    .sample_type = GPU::TextureSampleType::Float{},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 4,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture{
                    .sample_type = GPU::TextureSampleType::Float{},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 5,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture{
                    .sample_type = GPU::TextureSampleType::Float{},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 6,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Texture{
                    .sample_type = GPU::TextureSampleType::Float{},
                    .view_dimension = GPU::TextureViewDimension::D2,
                    .multi_sampled = false,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 7,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Sampler{
                    .type = GPU::SamplerBindingType::Filtering,
                },
                .count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .binding = 8,
                .visibility = GPU::ShaderStage::Fragment,
                .type = GPU::BindingType::Sampler{
                    .type = GPU::SamplerBindingType::Filtering,
                },
                .count = 1,
            },

        };

        GPU::BindGroupLayoutSpec spec{
            .label = "Object BGL"sv,
            .entries = entries,
        };

        m_object_bind_group_layout = Renderer::device().create_bind_group_layout(spec);
    }

    ////////////////////////
    /// RENDER PASS CREATION
    ////////////////////////

    {
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/simple.wgsl").value();

        GPU::ShaderModuleSpec shader_spec{
            .label = "Simple WGSL Shader"sv,
            .type = GPU::WGSLShader{
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts{
            m_global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec{
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        GPU::RenderPipelineSpec rp_spec{
            .label = "Simple RP"sv,
            .layout = layout,
            .vertex = {},
            .primitive = GPU::PrimitiveState::get_default(),
            .depth_stencil = None(),
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = GPU::FragmentStage{
                .targets = {
                    GPU::ColorTargetState{
                        .format = GPU::TextureFormat::RGBA8Unorm,
                        .blend = GPU::BlendState::get_default(),
                        .write_mask = GPU::ColorWrite::All,
                    }
                }
            },
        };

        m_simple_pipeline = Renderer::device().create_render_pipeline(shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/PBR.wgsl").value();

        GPU::ShaderModuleSpec shader_spec{
            .label = "PBR Shader"sv,
            .type = GPU::WGSLShader{
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts{
            m_global_bind_group_layout,
            m_object_bind_group_layout,
        };
        GPU::PipelineLayoutSpec pl_spec{
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        std::array attributes{
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float3,
                .shader_location = 0,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float3,
                .shader_location = 1,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float4,
                .shader_location = 2,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float2,
                .shader_location = 3,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float3,
                .shader_location = 4,
            },
        };
        auto attribute_layout = GPU::VertexBufferLayout::create(attributes);

        GPU::RenderPipelineSpec rp_spec{
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
            .fragment = GPU::FragmentStage{
                .targets = {
                    GPU::ColorTargetState{
                        .format = HDRPipeline::Format,
                        .blend = GPU::BlendState::get_default(),
                        .write_mask = GPU::ColorWrite::All,
                    },
                }
            },
        };

        m_pbr_pipeline = Renderer::device().create_render_pipeline(shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/Editor/Grid.wgsl").value();

        GPU::ShaderModuleSpec shader_spec{
            .label = "Grid Shader"sv,
            .type = GPU::WGSLShader{
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts{
            m_global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec{
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        GPU::RenderPipelineSpec rp_spec{
            .label = "Grid RP"sv,
            .layout = layout,
            .vertex = {},
            .primitive = GPU::PrimitiveState::get_default(),
            .depth_stencil = GPU::DepthStencilState::get_default(),
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = GPU::FragmentStage{
                .targets = {
                    GPU::ColorTargetState{
                        .format = HDRPipeline::Format,
                        .blend = GPU::BlendState::get_default(),
                        .write_mask = GPU::ColorWrite::All,
                    }
                }
            },
        };

        m_grid_pipeline = Renderer::device().create_render_pipeline(shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/Sky.wgsl").value();

        GPU::ShaderModuleSpec shader_spec{
            .label = "Sky Shader"sv,
            .type = GPU::WGSLShader{
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            .fragment_entry_point = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bind_group_layouts{
            m_global_bind_group_layout
        };
        GPU::PipelineLayoutSpec pl_spec{
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        auto primitive = GPU::PrimitiveState::get_default();
        primitive.topology = GPU::PrimitiveTopology::TriangleStrip;

        auto depth = GPU::DepthStencilState::get_default();
        depth.depth_write_enabled = false;
        depth.depth_compare = GPU::CompareFunction::Always;
        GPU::RenderPipelineSpec rp_spec{
            .label = "Sky RP"sv,
            .layout = layout,
            .vertex = {},
            .primitive = primitive,
            .depth_stencil = depth,
            .multi_sample = GPU::MultiSampleState::get_default(),
            .fragment = GPU::FragmentStage{
                .targets = {
                    GPU::ColorTargetState{
                        .format = HDRPipeline::Format,
                        .blend = GPU::BlendState::get_default(),
                        .write_mask = GPU::ColorWrite::All,
                    }
                }
            },
        };

        m_sky_pipeline = Renderer::device().create_render_pipeline(shader, rp_spec);
    }

    GPU::BufferSpec ibs{
        .label = "PBR Instance Buffer"sv,
        .usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        .size = sizeof(Mat4) * 2'000,
        .mapped = false,
    };
    m_pbr_instance_buffer = Renderer::device().create_buffer(ibs);

    m_pbr_instance_staging_buffer.reserve(sizeof(Mat4) * 2'000);

    GPU::SamplerSpec bilinear_sampler_spec{
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
    bilinear_sampler_spec.mag_filter = GPU::FilterMode::Nearest;
    bilinear_sampler_spec.min_filter = GPU::FilterMode::Nearest;
    bilinear_sampler_spec.anisotropy_clamp = 1_u16;

    m_shadow_sampler = Renderer::device().create_sampler(bilinear_sampler_spec);

    m_hdr_pipeline.init(window_size, m_scene_render_target.spec.format);

    Debug::initialize(Renderer::device(), m_global_bind_group_layout, m_hdr_pipeline.Format);

    setup_shadow_pass();
}

void SceneRenderer::resize(Vector2 const& new_size)
{
    ZoneScoped;
    m_render_area = new_size;

    m_scene_render_target.release();
    m_scene_render_depth_target.release();
    create_scene_render_target(new_size);
    m_hdr_pipeline.resize(new_size);
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
    scene_view_data.data.position = packet.camera.position;
    scene_view_data.flush();

    if (!m_render_context.directional_lights.empty()) {
        scene_light_data.data.directional_light = m_render_context.directional_lights[0].shader_data;
        scene_light_data.data.shadow_split_distances = Vector4{ 0.0f };
    }
    scene_light_data.flush();

    depth_pass(encoder, packet);
    pbr_pass(encoder, packet, game_view);

    m_hdr_pipeline.process(encoder, m_scene_render_target.view);

    for (auto& group : m_object_groups_to_release) {
        group.release();
    }
    m_object_groups_to_release.clear();

    Debug::reset();
    /* #if 0
        using namespace Fussion;
        auto current_frame = Renderer::GetInstance()->GetSwapchain()->GetCurrentFrame();
        m_CurrentFrame = current_frame;

        m_RenderContext.Reset();
        m_FrameAllocator.Reset();

        // TODO: Hack
        SceneGlobalData.Data.Time += Time::DeltaTime();
        SceneGlobalData.Flush();

        SceneViewData.Data.Perspective = packet.Camera.Perspective;
        SceneViewData.Data.View = packet.Camera.View;
        SceneViewData.Flush();

        SceneSceneData.Data.ViewPosition = packet.Camera.Position;
        // m_SceneData.Data.ViewDirection = packet.Camera.Direction;
        SceneSceneData.Data.AmbientColor = Color::White;
        SceneSceneData.Flush();

        SceneDebugOptions.Flush();

        m_RenderContext.Cmd = cmd;
        m_RenderContext.DirectionalLights.clear();
        m_RenderContext.PointLights.clear();

        cmd->BindUniformBuffer(SceneViewData.GetBuffer(), m_GlobalResource[current_frame], 0);
        cmd->BindUniformBuffer(SceneDebugOptions.GetBuffer(), m_GlobalResource[current_frame], 1);
        cmd->BindUniformBuffer(SceneGlobalData.GetBuffer(), m_GlobalResource[current_frame], 2);

        cmd->BindUniformBuffer(SceneSceneData.GetBuffer(), m_SceneResource[current_frame], 0);
        cmd->BindUniformBuffer(SceneLightData.GetBuffer(), m_SceneResource[current_frame], 1);

        {
            ZoneScopedN("Render Object Collection");
            m_RenderContext.RenderFlags = RenderState::LightCollection;
            if (packet.Scene) {
                packet.Scene->ForEachEntity([&](Entity* entity) {
                    entity->OnDraw(m_RenderContext);
                });
            }

            // {
            //     ZoneScopedN("Sorting");
            //     std::ranges::sort(m_RenderContext.RenderObjects, [](RenderObject const& first, RenderObject const& second) {
            //         return first.IndexBuffer < second.IndexBuffer;
            //     });
            // }
        }

        if (!m_RenderContext.DirectionalLights.empty()) {
            SceneLightData.Data.DirectionalLight = m_RenderContext.DirectionalLights[0].ShaderData;
        }
        SceneLightData.Flush();

        {
            ZoneScopedN("Depth Pass");
            m_RenderContext.RenderFlags = RenderState::Depth;

            u32 buffer_offset = 0;
            for (auto& light : m_RenderContext.DirectionalLights) {
                cmd->TransitionImageLayout(m_DepthImage, ImageLayout::ShaderReadOnlyOptimal, ImageLayout::DepthStencilReadOnlyOptimal);

                std::array<f32, ShadowCascades> shadow_splits{};
                for (auto i = 0; i < ShadowCascades; i++) {
                    // SceneLightData.Data.ShadowSplitDistances[i] = GetSplitDepth(i + 1, ShadowCascades, packet.Camera.Near, packet.Camera.Far, light.Split);
                    shadow_splits[i] = GetSplitDepth(i + 1, ShadowCascades, packet.Camera.Near, packet.Camera.Far, light.Split);
                }

                cmd->SetViewport({ ShadowMapResolution, -ShadowMapResolution });
                cmd->SetScissor(Vector4(0, 0, ShadowMapResolution, ShadowMapResolution));

                f32 last_split{ 0 };
                for (auto i = 0; i < ShadowCascades; i++) {
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
                    glm::mat4 inv_cam = glm::inverse(packet.Camera.Perspective * packet.Camera.View);
                    for (auto& frustum_corner : frustum_corners) {
                        Vector4 inv_corner = inv_cam * Vector4(frustum_corner, 1.0f);
                        frustum_corner = inv_corner / inv_corner.W;
                    }

                    auto split = shadow_splits[i];
                    for (u32 j = 0; j < 4; j++) {
                        Vector3 dist = frustum_corners[j + 4] - frustum_corners[j];
                        frustum_corners[j + 4] = frustum_corners[j] + dist * split;
                        frustum_corners[j] = frustum_corners[j] + dist * last_split;
                    }

                    Vector3 center{};
                    for (auto frustum_corner : frustum_corners) {
                        center += frustum_corner;
                    }
                    center /= 8.0f;

                    float radius = 0.0f;
                    for (auto frustum_corner : frustum_corners) {
                        f32 distance = (frustum_corner - center).Length();
                        radius = Math::Max(radius, distance);
                    }
                    radius = std::ceil(radius * 16.0f) / 16.0f;

                    Vector3 max_extents{ radius, radius, radius };
                    Vector3 min_extents = -max_extents;

                    glm::mat4 view = glm::lookAt(glm::vec3(center - Vector3(light.ShaderData.Direction) * min_extents.Z), glm::vec3(center), glm::vec3(Vector3::Up));
                    glm::mat4 proj = glm::ortho(min_extents.X, max_extents.X, min_extents.Y, max_extents.Y, -10.0f, max_extents.Z - min_extents.Z);

                    light.ShaderData.LightSpaceMatrix[i] = proj * view;

                    cmd->BeginRenderPass(m_DepthPass, m_ShadowFrameBuffers[i]);
                    auto depth_shader = m_DepthShader.Get()->GetShader();
                    cmd->UseShader(depth_shader);
                    SceneLightData.Data.ShadowSplitDistances[i] = packet.Camera.Near + split * (packet.Camera.Far - packet.Camera.Near) * -1.0f;
                    SceneLightData.Data.DirectionalLight.LightSpaceMatrix[i] = light.ShaderData.LightSpaceMatrix[i];

                    // for (auto const& obj : m_RenderContext.RenderObjects) {
                    //     struct PushData {
                    //         Mat4 Model, LightSpace;
                    //     } push;
                    //     push.Model = obj.WorldMatrix;
                    //     push.LightSpace = light.ShaderData.LightSpaceMatrix[i];
                    //     cmd->PushConstants(depth_shader, &push);
                    //
                    //     cmd->BindBuffer(obj.VertexBuffer);
                    //     cmd->BindBuffer(obj.IndexBuffer);
                    //     cmd->DrawIndexed(obj.IndexCount, 1);
                    // }
                    for (auto const& [buffer, list] : m_RenderContext.MeshRenderLists) {
                        ZoneScopedN("Command Buffer Dispatch");
                        (void)buffer;
                        auto object_layout = Device::Instance()->CreateResourceLayout(m_ObjectDepthResourceUsages);
                        auto resource = m_FrameAllocator.Alloc(object_layout, "Depth Object Instance Data");
                        cmd->BindResource(resource, depth_shader, 0);

                        auto const& hack = m_RenderContext.RenderObjects[list[0]];
                        struct InstanceData {
                            Mat4 Model;
                            Mat4 LightSpace;
                        };

                        auto data = CAST(InstanceData*, m_InstanceBuffers[current_frame]->GetMappedData()) + buffer_offset;
                        int j = 0;
                        for (auto index : list) {
                            // Fill vertex buffer with transforms
                            auto& obj = m_RenderContext.RenderObjects[index];
                            data[j].Model = obj.WorldMatrix;
                            data[j++].LightSpace = light.ShaderData.LightSpaceMatrix[i];
                            // transforms.push_back(obj.WorldMatrix);
                            // transforms.push_back(light.ShaderData.LightSpaceMatrix[i]);
                        }
                        // m_InstanceBuffers[current_frame]->SetData(std::span(transforms));

                        // Access the first render object from out index list in order to
                        // get access to the mesh. This is a hack. Probably the render
                        // list should include a pointer to the mesh.
                        // hack.InstanceBuffer->SetData(std::span(transforms));

                        cmd->BindBuffer(hack.IndexBuffer);
                        cmd->BindBuffer(hack.VertexBuffer);
                        cmd->BindStorageBuffer(m_InstanceBuffers[current_frame], resource, 0);

                        cmd->DrawIndexed(hack.IndexCount, list.size(), buffer_offset);
                        buffer_offset += list.size();
                    }
                    // if (packet.Scene) {
                    //     m_RenderContext.CurrentShader = depth_shader;
                    //     m_RenderContext.CurrentLightSpace = light.ShaderData.LightSpaceMatrix[i];
                    //     packet.Scene->ForEachEntity([&](Entity* entity) {
                    //         ZoneScopedN("Entity Draw");
                    //         entity->OnDraw(m_RenderContext);
                    //     });
                    // }
                    cmd->EndRenderPass(m_DepthPass);
                    last_split = split;
                }
                SceneLightData.Flush();
                cmd->TransitionImageLayout(m_DepthImage, ImageLayout::DepthStencilReadOnlyOptimal, ImageLayout::ShaderReadOnlyOptimal);
            }
        }

        cmd->BindImage(m_DepthImage, m_SceneResource[current_frame], 2);

        if (!m_RenderContext.DirectionalLights.empty()) {
            ZoneScopedN("ShadowMap Debug Viewer");
            cmd->SetViewport({ ShadowMapResolution, -ShadowMapResolution });
            cmd->SetScissor(Vector4(0, 0, ShadowMapResolution, ShadowMapResolution));

            cmd->BeginRenderPass(m_ShadowPassDebugRenderPass, m_ShadowViewerFrameBuffers[current_frame]);
            {

                auto shader = m_DepthViewerShader.Get()->GetShader();
                cmd->UseShader(shader);
                cmd->BindResource(m_SceneResource[current_frame], shader, 1);
                struct {
                    s32 CascadeIndex;
                } pc;
                pc.CascadeIndex = RenderDebugOptions.CascadeIndex;
                cmd->PushConstants(shader, &pc, sizeof(pc));
                cmd->Draw(6, 1);
            }
            cmd->EndRenderPass(m_ShadowPassDebugRenderPass);
        }

        cmd->SetViewport({ m_RenderArea.X, -m_RenderArea.Y });
        cmd->SetScissor(Vector4(0, 0, m_RenderArea.X, m_RenderArea.Y));

        // cmd->BeginRenderPass(m_ObjectPickingRenderPass, m_ObjectPickingFrameBuffer);
        // {
        //     ZoneScopedN("Object Picking Render Pass");
        //     auto object_pick_shader = m_ObjectPickingShader.Get()->GetShader();
        //
        //     m_RenderContext.RenderFlags = RenderState::ObjectPicking;
        //     m_RenderContext.CurrentPass = m_ObjectPickingRenderPass;
        //     m_RenderContext.CurrentShader = object_pick_shader;
        //
        //     cmd->UseShader(object_pick_shader);
        //     cmd->BindResource(m_GlobalResource[current_frame], object_pick_shader, 0);
        //
        //     // if (packet.Scene) {
        //     //     packet.Scene->ForEachEntity([&](Entity* entity) {
        //     //         ZoneScopedN("Object Picking Draw");
        //     //         entity->OnDraw(m_RenderContext);
        //     //     });
        //     // }
        // }
        // cmd->EndRenderPass(m_ObjectPickingRenderPass);

        cmd->BeginRenderPass(m_SceneRenderPass, m_FrameBuffers[current_frame]);
        {
            m_RenderContext.CurrentPass = m_SceneRenderPass;

            ZoneScopedN("Scene Render Pass");
            cmd->SetViewport({ m_RenderArea.X, -m_RenderArea.Y });
            cmd->SetScissor(Vector4(0, 0, m_RenderArea.X, m_RenderArea.Y));

            {
                ZoneScopedN("Skybox");

                auto sky_shader = m_SkyShader.Get()->GetShader();
                cmd->UseShader(sky_shader);
                cmd->BindResource(m_GlobalResource[current_frame], sky_shader, 0);
                cmd->BindResource(m_SceneResource[current_frame], sky_shader, 1);
                cmd->Draw(4, 1);
            }

            {
                ZoneScopedN("PBR");
                m_RenderContext.RenderFlags = RenderState::Mesh;
                auto pbr_shader = m_PbrShader.Get()->GetShader();
                cmd->UseShader(pbr_shader);
                cmd->BindResource(m_GlobalResource[current_frame], pbr_shader, 0);
                cmd->BindResource(m_SceneResource[current_frame], pbr_shader, 1);

                u32 buffer_offset = 0;
                for (auto const& [buffer, list] : m_RenderContext.MeshRenderLists) {
                    ZoneScopedN("PBR Command Buffer Dispatch");
                    (void)buffer;
                    auto object_layout = Device::Instance()->CreateResourceLayout(m_PBRResourceUsages);
                    auto resource = m_FrameAllocator.Alloc(object_layout, "PBR Object Instance Data");
                    cmd->BindResource(resource, pbr_shader, 2);

                    auto const& hack = m_RenderContext.RenderObjects[list[0]];

                    hack.Material->MaterialUniformBuffer.Data.ObjectColor = hack.Material->ObjectColor;
                    hack.Material->MaterialUniformBuffer.Data.Metallic = hack.Material->Metallic;
                    hack.Material->MaterialUniformBuffer.Data.Roughness = hack.Material->Roughness;
                    hack.Material->MaterialUniformBuffer.Flush();

                    cmd->BindUniformBuffer(hack.Material->MaterialUniformBuffer.GetBuffer(), resource, 0);
                    auto albedo = hack.Material->AlbedoMap.Get();
                    if (!albedo) {
                        albedo = Renderer::WhiteTexture().Get();
                    }

                    auto normal = hack.Material->NormalMap.Get();
                    if (!normal) {
                        normal = Renderer::DefaultNormalMap().Get();
                    }

                    auto ao = hack.Material->AmbientOcclusionMap.Get();
                    if (!ao) {
                        ao = Renderer::WhiteTexture().Get();
                    }

                    auto metallic_roughness = hack.Material->MetallicRoughnessMap.Get();
                    if (!metallic_roughness) {
                        metallic_roughness = Renderer::WhiteTexture().Get();
                    }

                    auto emissive = hack.Material->EmissiveMap.Get();
                    if (!emissive) {
                        emissive = Renderer::BlackTexture().Get();
                    }

                    cmd->BindImage(albedo->GetImage(), resource, 1);
                    cmd->BindImage(normal->GetImage(), resource, 2);

                    cmd->BindImage(ao->GetImage(), resource, 3);
                    cmd->BindImage(metallic_roughness->GetImage(), resource, 4);
                    cmd->BindImage(emissive->GetImage(), resource, 5);

                    struct InstanceData {
                        Mat4 Model;
                    };

                    auto data = CAST(InstanceData*, m_PBRInstanceBuffers[current_frame]->GetMappedData()) + buffer_offset;
                    int j = 0;
                    for (auto index : list) {
                        auto& obj = m_RenderContext.RenderObjects[index];
                        data[j++].Model = obj.WorldMatrix;
                    }

                    cmd->BindBuffer(hack.IndexBuffer);
                    cmd->BindBuffer(hack.VertexBuffer);
                    cmd->BindStorageBuffer(m_PBRInstanceBuffers[current_frame], resource, 6);

                    cmd->DrawIndexed(hack.IndexCount, list.size(), buffer_offset);
                    buffer_offset += list.size();
                }
            }

            if (!game_view) {
                {
                    ZoneScopedN("Debug Draw");
                    Debug::Render(cmd, m_GlobalResource[current_frame]);
                }

                {
                    ZoneScopedN("Editor Grid");
                    auto grid_shader = m_GridShader.Get()->GetShader();
                    m_RenderContext.CurrentShader = grid_shader;
                    cmd->UseShader(grid_shader);
                    cmd->BindResource(m_GlobalResource[current_frame], grid_shader, 0);
                    // cmd->BindResource(m_SceneResource, m_GridShader, 1);
                    cmd->Draw(6, 1);
                }
            }
            Debug::Reset();

        }
        cmd->EndRenderPass(m_SceneRenderPass);
    #endif */
}

void SceneRenderer::setup_shadow_pass_render_target()
{
    GPU::TextureSpec spec{
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
        auto shader_src = GPU::ShaderProcessor::process_file("Assets/Shaders/WGSL/DepthPass.wgsl").value();

        GPU::ShaderModuleSpec shader_spec{
            .label = "PBR Shader"sv,
            .type = GPU::WGSLShader{
                .source = shader_src,
            },
            .vertex_entry_point = "vs_main",
            //.FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::device().create_shader_module(shader_spec);

        std::array bgl_entries{
            GPU::BindGroupLayoutEntry{
                .binding = 0,
                .visibility = GPU::ShaderStage::Vertex,
                .type = GPU::BindingType::Buffer{
                    .type = GPU::BufferBindingType::Storage{
                        .read_only = true,
                    },
                },
                .count = 1,
            },
        };
        GPU::BindGroupLayoutSpec bgl_spec{
            .entries = bgl_entries,
        };

        m_object_depth_bgl = Renderer::device().create_bind_group_layout(bgl_spec);

        std::array bind_group_layouts{
            m_object_depth_bgl,
        };
        GPU::PipelineLayoutSpec pl_spec{
            .bind_group_layouts = bind_group_layouts
        };
        auto layout = Renderer::device().create_pipeline_layout(pl_spec);

        std::array attributes{
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float3,
                .shader_location = 0,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float3,
                .shader_location = 1,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float4,
                .shader_location = 2,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float2,
                .shader_location = 3,
            },
            GPU::VertexAttribute{
                .type = GPU::ElementType::Float3,
                .shader_location = 4,
            },
        };
        auto attribute_layout = GPU::VertexBufferLayout::create(attributes);

        GPU::RenderPipelineSpec rp_spec{
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

        m_depth_pipeline = Renderer::device().create_render_pipeline(shader, rp_spec);
    }

    GPU::BufferSpec ibs{
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

            std::array<f32, MAX_SHADOW_CASCADES> shadow_splits{};
            for (auto i = 0; i < MAX_SHADOW_CASCADES; i++) {
                // SceneLightData.Data.ShadowSplitDistances[i] = GetSplitDepth(i + 1, ShadowCascades, packet.Camera.Near, packet.Camera.Far, light.Split);
                shadow_splits[i] = GetSplitDepth(i + 1, MAX_SHADOW_CASCADES, packet.camera.near, packet.camera.far, light.split);
            }

            GPU::Buffer instance_buffer;
            if (!m_instance_buffer_pool.empty()) {
                instance_buffer = m_instance_buffer_pool.back();
                m_instance_buffer_pool.pop_back();
            } else {
                GPU::BufferSpec spec{
                    .label = "Depth Instance Buffer"sv,
                    .usage = GPU::BufferUsage::CopySrc | GPU::BufferUsage::MapWrite,
                    .size = sizeof(Mat4) * 2 * 2'000,
                    .mapped = true,
                };
                instance_buffer = Renderer::device().create_buffer(spec);
            }

            f32 last_split{ 0 };
            sz buffer_offset = 0;
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

                Vector3 center{};
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

                Vector3 max_extents{ radius, radius, radius };
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

                for (auto const& [buffer, list] : m_render_context.mesh_render_lists) {
                    auto data = TRANSMUTE(DepthInstanceData*, instance_buffer.slice().mapped_range()) + buffer_offset;
                    int j = 0;
                    for (auto index : list) {
                        auto& obj = m_render_context.render_objects[index];
                        data[j].model = obj.world_matrix;
                        data[j++].light_space = light.shader_data.light_space_matrix[i];
                    }

                    buffer_offset += list.size();
                }
                last_split = split;
            }

            instance_buffer.unmap();
            auto copy_encoder = Renderer::device().create_command_encoder();

            copy_encoder.copy_buffer_to_buffer(instance_buffer, 0, m_depth_instance_buffer, 0, buffer_offset * sizeof(DepthInstanceData));
            Renderer::device().submit_command_buffer(copy_encoder.finish());
            copy_encoder.release();

            instance_buffer.slice().map_async([instance_buffer, this] {
                m_instance_buffer_pool.push_back(instance_buffer);
            });

            buffer_offset = 0;
            for (auto i = 0; i < MAX_SHADOW_CASCADES; i++) {
                GPU::RenderPassSpec rp_spec{
                    .label = "DepthPass::RenderPass"sv,
                    .depth_stencil_attachment = GPU::RenderPassColorAttachment{
                        .view = m_shadow_pass_render_target_views[i],
                        .load_op = GPU::LoadOp::Clear,
                        .store_op = GPU::StoreOp::Store,
                        .depth_clear = 1.0f,
                    },
                };
                auto rp = encoder.begin_rendering(rp_spec);
                rp.set_viewport({}, { SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION });
                rp.set_pipeline(m_depth_pipeline);

                for (auto const& [buffer, list] : m_render_context.mesh_render_lists) {
                    ZoneScopedN("Command Buffer Dispatch");
                    (void)buffer;

                    auto const& hack = m_render_context.render_objects[list[0]];

                    std::array bind_group_entries{
                        GPU::BindGroupEntry{
                            .binding = 0,
                            .resource = GPU::BufferBinding{
                                .buffer = m_depth_instance_buffer,
                                .offset = 0,
                                .size = m_depth_instance_buffer.size(),
                            }
                        },
                    };

                    GPU::BindGroupSpec bg_spec{
                        .label = "Object Depth Bind Group"sv,
                        .entries = bind_group_entries
                    };

                    auto object_group = Renderer::device().create_bind_group(m_object_depth_bgl, bg_spec);
                    m_object_groups_to_release.push_back(object_group);

                    rp.set_vertex_buffer(0, hack.vertex_buffer);
                    rp.set_index_buffer(hack.index_buffer);
                    rp.set_bind_group(object_group, 0);

                    rp.draw_index({ 0, hack.index_count }, { CAST(u32, buffer_offset), CAST(u32, list.size()) });
                    buffer_offset += list.size();
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
    ZoneScopedN("PBR Pass");
    std::array color_attachments{
        GPU::RenderPassColorAttachment{
            .view = m_hdr_pipeline.view(),
            .load_op = GPU::LoadOp::Clear,
            .store_op = GPU::StoreOp::Store,
            .clear_color = Color::Black,
        },
    };

    GPU::RenderPassSpec scene_rp_spec{
        .label = "Scene Render Pass"sv,
        .color_attachments = color_attachments,
        .depth_stencil_attachment = GPU::RenderPassColorAttachment{
            .view = m_scene_render_depth_target.view,
            .load_op = GPU::LoadOp::Clear,
            .store_op = GPU::StoreOp::Store,
            .depth_clear = 1.0f,
        },
    };
    auto scene_rp = encoder.begin_rendering(scene_rp_spec);

    scene_rp.set_viewport(Vector2::Zero, { m_render_area.x, m_render_area.y });

    scene_rp.set_bind_group(m_global_bind_group, 0);
    {
        scene_rp.set_pipeline(m_sky_pipeline);
        scene_rp.draw({ 0, 4 }, { 0, 1 });
    }

    scene_rp.set_pipeline(m_pbr_pipeline);

    GPU::Buffer instance_buffer;
    if (!m_instance_buffer_pool.empty()) {
        instance_buffer = m_instance_buffer_pool.back();
        m_instance_buffer_pool.pop_back();
    } else {
        GPU::BufferSpec spec{
            .label = "PBR Instance Buffer"sv,
            .usage = GPU::BufferUsage::CopySrc | GPU::BufferUsage::MapWrite,
            .size = sizeof(Mat4) * 2'000,
            .mapped = true,
        };
        instance_buffer = Renderer::device().create_buffer(spec);
    }

    sz buffer_count_offset = 0;

    for (auto const& [buffer, list] : m_render_context.mesh_render_lists) {
        auto data = TRANSMUTE(InstanceData*, instance_buffer.slice().mapped_range()) + buffer_count_offset;
        int j = 0;
        for (auto index : list) {
            auto& obj = m_render_context.render_objects[index];
            data[j++].model = obj.world_matrix;
        }

        buffer_count_offset += list.size();
    }
    instance_buffer.unmap();

    auto copy_encoder = Renderer::device().create_command_encoder();

    copy_encoder.copy_buffer_to_buffer(instance_buffer, 0, m_pbr_instance_buffer, 0, buffer_count_offset * sizeof(InstanceData));
    Renderer::device().submit_command_buffer(copy_encoder.finish());
    copy_encoder.release();

    instance_buffer.slice().map_async([instance_buffer, this] {
        m_instance_buffer_pool.push_back(instance_buffer);
    });

    buffer_count_offset = 0;
    for (auto const& [buffer, list] : m_render_context.mesh_render_lists) {
        ZoneScopedN("PBR Command Buffer Dispatch");
        (void)buffer;

        auto const& hack = m_render_context.render_objects[list[0]];

        hack.material->material_uniform_buffer.data.object_color = hack.material->object_color;
        hack.material->material_uniform_buffer.data.metallic = hack.material->metallic;
        hack.material->material_uniform_buffer.data.roughness = hack.material->roughness;
        hack.material->material_uniform_buffer.flush();

        auto albedo = hack.material->albedo_map.get();
        if (!albedo) {
            albedo = Renderer::white_texture().get();
        }

        auto normal = hack.material->normal_map.get();
        if (!normal) {
            normal = Renderer::default_normal_map().get();
        }

        auto ao = hack.material->ambient_occlusion_map.get();
        if (!ao) {
            ao = Renderer::white_texture().get();
        }

        auto metallic_roughness = hack.material->metallic_roughness_map.get();
        if (!metallic_roughness) {
            metallic_roughness = Renderer::white_texture().get();
        }

        auto emissive = hack.material->emissive_map.get();
        if (!emissive) {
            emissive = Renderer::black_texture().get();
        }

        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .binding = 0,
                .resource = GPU::BufferBinding{
                    .buffer = m_pbr_instance_buffer,
                    .offset = 0,
                    .size = m_pbr_instance_buffer.size(),
                }
            },
            GPU::BindGroupEntry{
                .binding = 1, .resource = GPU::BufferBinding{
                    .buffer = hack.material->material_uniform_buffer.buffer(),
                    .offset = 0,
                    .size = hack.material->material_uniform_buffer.buffer().size(),
                }
            },
            GPU::BindGroupEntry{
                .binding = 2,
                .resource = albedo->image().view,
            },
            GPU::BindGroupEntry{
                .binding = 3,
                .resource = normal->image().view,
            },
            GPU::BindGroupEntry{
                .binding = 4,
                .resource = metallic_roughness->image().view,
            },
            GPU::BindGroupEntry{
                .binding = 5,
                .resource = ao->image().view,
            },
            GPU::BindGroupEntry{
                .binding = 6,
                .resource = emissive->image().view,
            },
            GPU::BindGroupEntry{
                .binding = 7,
                .resource = m_linear_sampler,
            },
            GPU::BindGroupEntry{
                .binding = 8,
                .resource = m_shadow_sampler,
            },
        };

        GPU::BindGroupSpec global_bg_spec{
            .label = "Object Bind Group"sv,
            .entries = bind_group_entries
        };

        auto object_group = Renderer::device().create_bind_group(m_object_bind_group_layout, global_bg_spec);
        m_object_groups_to_release.push_back(object_group);

        scene_rp.set_vertex_buffer(0, hack.vertex_buffer);
        scene_rp.set_index_buffer(hack.index_buffer);
        scene_rp.set_bind_group(object_group, 1);

        scene_rp.draw_index({ 0, hack.index_count }, { CAST(u32, buffer_count_offset), CAST(u32, list.size()) });
        buffer_count_offset += list.size();
    }

    // Draw editor specific stuff only if we are not rendering a game view.
    if (!game_view) {
        Debug::render(scene_rp);

        scene_rp.set_pipeline(m_grid_pipeline);
        scene_rp.draw({ 0, 6 }, { 0, 1 });
    }

    scene_rp.end();
    scene_rp.release();
}

void SceneRenderer::create_scene_render_target(Vector2 const& size)
{
    GPU::TextureSpec spec{
        .label = "Main Render Target"sv,
        .usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .dimension = GPU::TextureDimension::D2,
        .size = Vector3{ size, 1 },
        .format = GPU::TextureFormat::RGBA8Unorm,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::All,
    };
    m_scene_render_target = Renderer::device().create_texture(spec);
    m_scene_render_target.initialize_view();

    GPU::TextureSpec depth_spec{
        .label = "Main Render Depth Target"sv,
        .usage = GPU::TextureUsage::RenderAttachment,
        .dimension = GPU::TextureDimension::D2,
        .size = Vector3{ size, 1 },
        .format = GPU::TextureFormat::Depth24Plus,
        .sample_count = 1,
        .aspect = GPU::TextureAspect::DepthOnly,
    };
    m_scene_render_depth_target = Renderer::device().create_texture(depth_spec);
    m_scene_render_depth_target.initialize_view();
}
