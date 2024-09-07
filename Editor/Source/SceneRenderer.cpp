#include "EditorPCH.h"

#include "SceneRenderer.h"
#include "Project/Project.h"

#include <Fussion/Core/Application.h>
#include <Fussion/GPU/ShaderProcessor.h>
#include <Fussion/Rendering/Renderer.h>
#include <Fussion/Core/Time.h>
#include <Fussion/Debug/Debug.h>
#include <Fussion/OS/FileSystem.h>
#include <Fussion/RHI/Device.h>
#include <tracy/Tracy.hpp>

#undef far
#undef near
#undef min
#undef max

using namespace Fussion;
using namespace Fussion::RHI;

void SceneRenderer::Init()
{
    auto window_size = Application::Instance()->GetWindow().GetSize();

    CreateSceneRenderTarget(window_size);

    SceneViewData = UniformBuffer<ViewData>::Create(Renderer::Device(), std::string_view{ "View Data" });
    SceneLightData = UniformBuffer<LightData>::Create(Renderer::Device(), std::string_view{ "Light Data" });

    ///////////////////////
    /// BIND GROUP CREATION
    ///////////////////////

    SetupShadowPassRenderTarget();

    {
        std::array entries{
            GPU::BindGroupLayoutEntry{
                .Binding = 0,
                .Visibility = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Buffer{
                    .Type = GPU::BufferBindingType::Uniform{},
                    .HasDynamicOffset = false,
                    .MinBindingSize = None(),
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 1,
                .Visibility = GPU::ShaderStage::Vertex | GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Buffer{
                    .Type = GPU::BufferBindingType::Uniform{},
                    .HasDynamicOffset = false,
                    .MinBindingSize = None(),
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 2,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture{
                    .SampleType = GPU::TextureSampleType::Depth{},
                    .ViewDimension = GPU::TextureViewDimension::D2_Array,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
        };

        GPU::BindGroupLayoutSpec spec{
            .Label = "Global BGL"sv,
            .Entries = entries,
        };

        m_GlobalBindGroupLayout = Renderer::Device().CreateBindGroupLayout(spec);

        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .Binding = 0,
                .Resource = GPU::BufferBinding{
                    .Buffer = SceneViewData.GetBuffer(),
                    .Offset = 0,
                    .Size = SceneViewData.Size(),
                }
            },
            GPU::BindGroupEntry{
                .Binding = 1,
                .Resource = GPU::BufferBinding{
                    .Buffer = SceneLightData.GetBuffer(),
                    .Offset = 0,
                    .Size = SceneLightData.Size(),
                }
            },
            GPU::BindGroupEntry{
                .Binding = 2,
                .Resource = m_ShadowPassRenderTarget.View
            }
        };

        GPU::BindGroupSpec global_bg_spec{
            .Label = "Global Bind Group"sv,
            .Entries = bind_group_entries
        };

        m_GlobalBindGroup = Renderer::Device().CreateBindGroup(m_GlobalBindGroupLayout, global_bg_spec);
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
                .Binding = 0,
                .Visibility = GPU::ShaderStage::Vertex,
                .Type = GPU::BindingType::Buffer{
                    .Type = GPU::BufferBindingType::Storage{
                        .ReadOnly = true,
                    },
                    .HasDynamicOffset = false,
                    .MinBindingSize = None(),
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 1,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Buffer{
                    .Type = GPU::BufferBindingType::Uniform{},
                    .HasDynamicOffset = false,
                    .MinBindingSize = None(),
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 2,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture{
                    .SampleType = GPU::TextureSampleType::Float{},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 3,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture{
                    .SampleType = GPU::TextureSampleType::Float{},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 4,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture{
                    .SampleType = GPU::TextureSampleType::Float{},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 5,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture{
                    .SampleType = GPU::TextureSampleType::Float{},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 6,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Texture{
                    .SampleType = GPU::TextureSampleType::Float{},
                    .ViewDimension = GPU::TextureViewDimension::D2,
                    .MultiSampled = false,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 7,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Sampler{
                    .Type = GPU::SamplerBindingType::Filtering,
                },
                .Count = 1,
            },
            GPU::BindGroupLayoutEntry{
                .Binding = 8,
                .Visibility = GPU::ShaderStage::Fragment,
                .Type = GPU::BindingType::Sampler{
                    .Type = GPU::SamplerBindingType::Filtering,
                },
                .Count = 1,
            },

        };

        GPU::BindGroupLayoutSpec spec{
            .Label = "Object BGL"sv,
            .Entries = entries,
        };

        m_ObjectBindGroupLayout = Renderer::Device().CreateBindGroupLayout(spec);
    }

    ////////////////////////
    /// RENDER PASS CREATION
    ////////////////////////

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/simple.wgsl").Value();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "Simple WGSL Shader"sv,
            .Type = GPU::WGSLShader{
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts{
            m_GlobalBindGroupLayout
        };
        GPU::PipelineLayoutSpec pl_spec{
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        GPU::RenderPipelineSpec rp_spec{
            .Label = "Simple RP"sv,
            .Layout = layout,
            .Vertex = {},
            .Primitive = GPU::PrimitiveState::Default(),
            .DepthStencil = None(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = GPU::FragmentStage{
                .Targets = {
                    GPU::ColorTargetState{
                        .Format = GPU::TextureFormat::RGBA8Unorm,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    }
                }
            },
        };

        m_SimplePipeline = Renderer::Device().CreateRenderPipeline(shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/PBR.wgsl").Value();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "PBR Shader"sv,
            .Type = GPU::WGSLShader{
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts{
            m_GlobalBindGroupLayout,
            m_ObjectBindGroupLayout,
        };
        GPU::PipelineLayoutSpec pl_spec{
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        std::array attributes{
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 0,
            },
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 1,
            },
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float4,
                .ShaderLocation = 2,
            },
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float2,
                .ShaderLocation = 3,
            },
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 4,
            },
        };
        auto attribute_layout = GPU::VertexBufferLayout::Create(attributes);

        GPU::RenderPipelineSpec rp_spec{
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
            .Fragment = GPU::FragmentStage{
                .Targets = {
                    GPU::ColorTargetState{
                        .Format = HDRPipeline::Format,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    },
                }
            },
        };

        m_PbrPipeline = Renderer::Device().CreateRenderPipeline(shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/Editor/Grid.wgsl").Value();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "Grid Shader"sv,
            .Type = GPU::WGSLShader{
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts{
            m_GlobalBindGroupLayout
        };
        GPU::PipelineLayoutSpec pl_spec{
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        GPU::RenderPipelineSpec rp_spec{
            .Label = "Grid RP"sv,
            .Layout = layout,
            .Vertex = {},
            .Primitive = GPU::PrimitiveState::Default(),
            .DepthStencil = GPU::DepthStencilState::Default(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = GPU::FragmentStage{
                .Targets = {
                    GPU::ColorTargetState{
                        .Format = HDRPipeline::Format,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    }
                }
            },
        };

        m_GridPipeline = Renderer::Device().CreateRenderPipeline(shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/Sky.wgsl").Value();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "Sky Shader"sv,
            .Type = GPU::WGSLShader{
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            .FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bind_group_layouts{
            m_GlobalBindGroupLayout
        };
        GPU::PipelineLayoutSpec pl_spec{
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        auto primitive = GPU::PrimitiveState::Default();
        primitive.Topology = GPU::PrimitiveTopology::TriangleStrip;

        auto depth = GPU::DepthStencilState::Default();
        depth.DepthWriteEnabled = false;
        depth.DepthCompare = GPU::CompareFunction::Always;
        GPU::RenderPipelineSpec rp_spec{
            .Label = "Sky RP"sv,
            .Layout = layout,
            .Vertex = {},
            .Primitive = primitive,
            .DepthStencil = depth,
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = GPU::FragmentStage{
                .Targets = {
                    GPU::ColorTargetState{
                        .Format = HDRPipeline::Format,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    }
                }
            },
        };

        m_SkyPipeline = Renderer::Device().CreateRenderPipeline(shader, rp_spec);
    }

    GPU::BufferSpec ibs{
        .Label = "PBR Instance Buffer"sv,
        .Usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        .Size = sizeof(Mat4) * 2'000,
        .Mapped = false,
    };
    m_PbrInstanceBuffer = Renderer::Device().CreateBuffer(ibs);

    m_PbrInstanceStagingBuffer.reserve(sizeof(Mat4) * 2'000);

    GPU::SamplerSpec bilinear_sampler_spec{
        .Label = "Bilinear Sampler"sv,
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

    m_LinearSampler = Renderer::Device().CreateSampler(bilinear_sampler_spec);
    bilinear_sampler_spec.MagFilter = GPU::FilterMode::Nearest;
    bilinear_sampler_spec.MinFilter = GPU::FilterMode::Nearest;
    bilinear_sampler_spec.AnisotropyClamp = 1_u16;

    m_ShadowSampler = Renderer::Device().CreateSampler(bilinear_sampler_spec);

    m_HDRPipeline.Init(window_size, m_SceneRenderTarget.Spec.Format);

    Debug::Initialize(Renderer::Device(), m_GlobalBindGroupLayout, m_HDRPipeline.Format);

    SetupShadowPass();
}

void SceneRenderer::Resize(Vector2 const& new_size)
{
    ZoneScoped;
    m_RenderArea = new_size;

    m_SceneRenderTarget.Release();
    m_SceneRenderDepthTarget.Release();
    CreateSceneRenderTarget(new_size);
    m_HDRPipeline.Resize(new_size);
}

f32 GetSplitDepth(s32 current_split, s32 max_splits, f32 near, f32 far, f32 l = 1.0f)
{
    auto split_ratio = CAST(f32, current_split) / CAST(f32, max_splits);
    auto log = near * Math::Pow(far / near, split_ratio);
    auto uniform = near + (far - near) * split_ratio;
    auto d = l * (log - uniform) + uniform;
    return (d - near) / (far - near);
}

void SceneRenderer::Render(GPU::CommandEncoder& encoder, RenderPacket const& packet, bool game_view)
{
    ZoneScoped;

    m_RenderContext.Reset();

    {
        ZoneScopedN("Render Object Collection");
        m_RenderContext.RenderFlags = RenderState::LightCollection;
        if (packet.Scene) {
            packet.Scene->ForEachEntity([&](Entity* entity) {
                entity->OnDraw(m_RenderContext);
            });
        }
    }

    SceneViewData.Data.Perspective = packet.Camera.Perspective;
    SceneViewData.Data.View = packet.Camera.View;
    SceneViewData.Data.Position = packet.Camera.Position;
    SceneViewData.Flush();

    if (!m_RenderContext.DirectionalLights.empty()) {
        SceneLightData.Data.DirectionalLight = m_RenderContext.DirectionalLights[0].ShaderData;
        SceneLightData.Data.ShadowSplitDistances = Vector4{ 0.0f };
    }
    SceneLightData.Flush();

    DepthPass(encoder, packet);
    PbrPass(encoder, packet, game_view);

    m_HDRPipeline.Process(encoder, m_SceneRenderTarget.View);

    for (auto& group : m_ObjectGroupsToRelease) {
        group.Release();
    }
    m_ObjectGroupsToRelease.clear();

    Debug::Reset();
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

void SceneRenderer::SetupShadowPassRenderTarget()
{
    GPU::TextureSpec spec{
        .Label = "DepthPass::RenderTarget"sv,
        .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = { ShadowMapResolution, ShadowMapResolution, ShadowCascades },
        .Format = GPU::TextureFormat::Depth24Plus,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::DepthOnly,
    };
    m_ShadowPassRenderTarget = Renderer::Device().CreateTexture(spec);
    m_ShadowPassRenderTarget.InitializeView(ShadowCascades);

    for (u32 i = 0; i < ShadowCascades; ++i) {
        m_ShadowPassRenderTargetViews[i] = m_ShadowPassRenderTarget.CreateView({
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

void SceneRenderer::SetupShadowPass()
{
    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/DepthPass.wgsl").Value();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "PBR Shader"sv,
            .Type = GPU::WGSLShader{
                .Source = shader_src,
            },
            .VertexEntryPoint = "vs_main",
            //.FragmentEntryPoint = "fs_main",
        };

        auto shader = Renderer::Device().CreateShaderModule(shader_spec);

        std::array bgl_entries{
            GPU::BindGroupLayoutEntry{
                .Binding = 0,
                .Visibility = GPU::ShaderStage::Vertex,
                .Type = GPU::BindingType::Buffer{
                    .Type = GPU::BufferBindingType::Storage{
                        .ReadOnly = true,
                    },
                },
                .Count = 1,
            },
        };
        GPU::BindGroupLayoutSpec bgl_spec{
            .Entries = bgl_entries,
        };

        m_ObjectDepthBGL = Renderer::Device().CreateBindGroupLayout(bgl_spec);

        std::array bind_group_layouts{
            m_ObjectDepthBGL,
        };
        GPU::PipelineLayoutSpec pl_spec{
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        std::array attributes{
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 0,
            },
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 1,
            },
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float4,
                .ShaderLocation = 2,
            },
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float2,
                .ShaderLocation = 3,
            },
            GPU::VertexAttribute{
                .Type = GPU::ElementType::Float3,
                .ShaderLocation = 4,
            },
        };
        auto attribute_layout = GPU::VertexBufferLayout::Create(attributes);

        GPU::RenderPipelineSpec rp_spec{
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

        m_DepthPipeline = Renderer::Device().CreateRenderPipeline(shader, rp_spec);
    }

    GPU::BufferSpec ibs{
        .Label = "Depth Instance Buffer"sv,
        .Usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        .Size = sizeof(Mat4) * 2'000,
        .Mapped = false,
    };
    m_DepthInstanceBuffer = Renderer::Device().CreateBuffer(ibs);

    m_DepthInstanceStagingBuffer.reserve(sizeof(Mat4) * 2'000);
}

void SceneRenderer::DepthPass(GPU::CommandEncoder& encoder, RenderPacket const& packet)
{
    {
        ZoneScopedN("Depth Pass");

        for (auto& light : m_RenderContext.DirectionalLights) {

            std::array<f32, ShadowCascades> shadow_splits{};
            for (auto i = 0; i < ShadowCascades; i++) {
                // SceneLightData.Data.ShadowSplitDistances[i] = GetSplitDepth(i + 1, ShadowCascades, packet.Camera.Near, packet.Camera.Far, light.Split);
                shadow_splits[i] = GetSplitDepth(i + 1, ShadowCascades, packet.Camera.Near, packet.Camera.Far, light.Split);
            }

            GPU::Buffer instance_buffer;
            if (!m_InstanceBufferPool.empty()) {
                instance_buffer = m_InstanceBufferPool.back();
                m_InstanceBufferPool.pop_back();
            } else {
                GPU::BufferSpec spec{
                    .Label = "Depth Instance Buffer"sv,
                    .Usage = GPU::BufferUsage::CopySrc | GPU::BufferUsage::MapWrite,
                    .Size = sizeof(Mat4) * 2 * 2'000,
                    .Mapped = true,
                };
                instance_buffer = Renderer::Device().CreateBuffer(spec);
            }

            f32 last_split{ 0 };
            u32 buffer_offset = 0;
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
                radius = std::ceil(radius * 16.f) / 16.f;

                auto texel_size = CAST(f32, ShadowMapResolution) / (radius * 2.0f);
                Mat4 scalar(texel_size);

                Vector3 max_extents{ radius, radius, radius };
                Vector3 min_extents = -max_extents;

                glm::mat4 view = glm::lookAt(glm::vec3(center - Vector3(light.ShaderData.Direction) * min_extents.Z), glm::vec3(center), glm::vec3(Vector3::Up));
                view = scalar * view;
                center = glm::mat3(view) * center;
                center.X = Math::Floor(center.X);
                center.Y = Math::Floor(center.Y);
                center = inverse(glm::mat3(view)) * center;
                view = glm::lookAt(glm::vec3(center - Vector3(light.ShaderData.Direction) * min_extents.Z), glm::vec3(center), glm::vec3(Vector3::Up));

                glm::mat4 proj = glm::ortho(min_extents.X, max_extents.X, min_extents.Y, max_extents.Y, -20.0f, max_extents.Z - min_extents.Z);

                light.ShaderData.LightSpaceMatrix[i] = proj * view;

                SceneLightData.Data.ShadowSplitDistances[i] = packet.Camera.Near + split * (packet.Camera.Far - packet.Camera.Near) * -1.0f;
                SceneLightData.Data.DirectionalLight.LightSpaceMatrix[i] = light.ShaderData.LightSpaceMatrix[i];

                for (auto const& [buffer, list] : m_RenderContext.MeshRenderLists) {
                    auto data = TRANSMUTE(DepthInstanceData*, instance_buffer.GetSlice().GetMappedRange()) + buffer_offset;
                    int j = 0;
                    for (auto index : list) {
                        auto& obj = m_RenderContext.RenderObjects[index];
                        data[j].Model = obj.WorldMatrix;
                        data[j++].LightSpace = light.ShaderData.LightSpaceMatrix[i];
                    }

                    buffer_offset += list.size();
                }
                last_split = split;
            }

            instance_buffer.UnMap();
            auto copy_encoder = Renderer::Device().CreateCommandEncoder();

            copy_encoder.CopyBufferToBuffer(instance_buffer, 0, m_DepthInstanceBuffer, 0, buffer_offset * sizeof(DepthInstanceData));
            Renderer::Device().SubmitCommandBuffer(copy_encoder.Finish());
            copy_encoder.Release();

            instance_buffer.GetSlice().MapAsync([instance_buffer, this] {
                m_InstanceBufferPool.push_back(instance_buffer);
            });

            buffer_offset = 0;
            for (auto i = 0; i < ShadowCascades; i++) {
                GPU::RenderPassSpec rp_spec{
                    .Label = "DepthPass::RenderPass"sv,
                    .DepthStencilAttachment = GPU::RenderPassColorAttachment{
                        .View = m_ShadowPassRenderTargetViews[i],
                        .LoadOp = GPU::LoadOp::Clear,
                        .StoreOp = GPU::StoreOp::Store,
                        .DepthClear = 1.0f,
                    },
                };
                auto rp = encoder.BeginRendering(rp_spec);
                rp.SetViewport({}, { ShadowMapResolution, ShadowMapResolution });
                rp.SetPipeline(m_DepthPipeline);

                for (auto const& [buffer, list] : m_RenderContext.MeshRenderLists) {
                    ZoneScopedN("Command Buffer Dispatch");
                    (void)buffer;

                    auto const& hack = m_RenderContext.RenderObjects[list[0]];

                    std::array bind_group_entries{
                        GPU::BindGroupEntry{
                            .Binding = 0,
                            .Resource = GPU::BufferBinding{
                                .Buffer = m_DepthInstanceBuffer,
                                .Offset = 0,
                                .Size = m_DepthInstanceBuffer.GetSize(),
                            }
                        },
                    };

                    GPU::BindGroupSpec bg_spec{
                        .Label = "Object Depth Bind Group"sv,
                        .Entries = bind_group_entries
                    };

                    auto object_group = Renderer::Device().CreateBindGroup(m_ObjectDepthBGL, bg_spec);
                    m_ObjectGroupsToRelease.push_back(object_group);

                    rp.SetVertexBuffer(0, hack.VertexBuffer);
                    rp.SetIndexBuffer(hack.IndexBuffer);
                    rp.SetBindGroup(object_group, 0);

                    rp.DrawIndex({ 0, hack.IndexCount }, { buffer_offset, CAST(u32, list.size()) });
                    buffer_offset += list.size();
                }
                rp.End();
                rp.Release();
            }
            SceneLightData.Flush();
        }
    }
}

void SceneRenderer::PbrPass(GPU::CommandEncoder& encoder, RenderPacket const& packet, bool game_view)
{
    ZoneScopedN("PBR Pass");
    std::array color_attachments{
        GPU::RenderPassColorAttachment{
            .View = m_HDRPipeline.View(),
            .LoadOp = GPU::LoadOp::Clear,
            .StoreOp = GPU::StoreOp::Store,
            .ClearColor = Color::Black,
        },
    };

    GPU::RenderPassSpec scene_rp_spec{
        .Label = "Scene Render Pass"sv,
        .ColorAttachments = color_attachments,
        .DepthStencilAttachment = GPU::RenderPassColorAttachment{
            .View = m_SceneRenderDepthTarget.View,
            .LoadOp = GPU::LoadOp::Clear,
            .StoreOp = GPU::StoreOp::Store,
            .DepthClear = 1.0f,
        },
    };
    auto scene_rp = encoder.BeginRendering(scene_rp_spec);

    scene_rp.SetViewport(Vector2::Zero, { m_RenderArea.X, m_RenderArea.Y });

    scene_rp.SetBindGroup(m_GlobalBindGroup, 0);
    {
        scene_rp.SetPipeline(m_SkyPipeline);
        scene_rp.Draw({ 0, 4 }, { 0, 1 });
    }

    scene_rp.SetPipeline(m_PbrPipeline);

    GPU::Buffer instance_buffer;
    if (!m_InstanceBufferPool.empty()) {
        instance_buffer = m_InstanceBufferPool.back();
        m_InstanceBufferPool.pop_back();
    } else {
        GPU::BufferSpec spec{
            .Label = "PBR Instance Buffer"sv,
            .Usage = GPU::BufferUsage::CopySrc | GPU::BufferUsage::MapWrite,
            .Size = sizeof(Mat4) * 2'000,
            .Mapped = true,
        };
        instance_buffer = Renderer::Device().CreateBuffer(spec);
    }

    u32 buffer_count_offset = 0;

    for (auto const& [buffer, list] : m_RenderContext.MeshRenderLists) {
        auto data = TRANSMUTE(InstanceData*, instance_buffer.GetSlice().GetMappedRange()) + buffer_count_offset;
        int j = 0;
        for (auto index : list) {
            auto& obj = m_RenderContext.RenderObjects[index];
            data[j++].Model = obj.WorldMatrix;
        }

        buffer_count_offset += list.size();
    }
    instance_buffer.UnMap();

    auto copy_encoder = Renderer::Device().CreateCommandEncoder();

    copy_encoder.CopyBufferToBuffer(instance_buffer, 0, m_PbrInstanceBuffer, 0, buffer_count_offset * sizeof(InstanceData));
    Renderer::Device().SubmitCommandBuffer(copy_encoder.Finish());
    copy_encoder.Release();

    instance_buffer.GetSlice().MapAsync([instance_buffer, this] {
        m_InstanceBufferPool.push_back(instance_buffer);
    });

    buffer_count_offset = 0;
    for (auto const& [buffer, list] : m_RenderContext.MeshRenderLists) {
        ZoneScopedN("PBR Command Buffer Dispatch");
        (void)buffer;

        auto const& hack = m_RenderContext.RenderObjects[list[0]];

        hack.Material->MaterialUniformBuffer.Data.ObjectColor = hack.Material->ObjectColor;
        hack.Material->MaterialUniformBuffer.Data.Metallic = hack.Material->Metallic;
        hack.Material->MaterialUniformBuffer.Data.Roughness = hack.Material->Roughness;
        hack.Material->MaterialUniformBuffer.Flush();

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

        std::array bind_group_entries{
            GPU::BindGroupEntry{
                .Binding = 0,
                .Resource = GPU::BufferBinding{
                    .Buffer = m_PbrInstanceBuffer,
                    .Offset = 0,
                    .Size = m_PbrInstanceBuffer.GetSize(),
                }
            },
            GPU::BindGroupEntry{
                .Binding = 1, .Resource = GPU::BufferBinding{
                    .Buffer = hack.Material->MaterialUniformBuffer.GetBuffer(),
                    .Offset = 0,
                    .Size = hack.Material->MaterialUniformBuffer.GetBuffer().GetSize(),
                }
            },
            GPU::BindGroupEntry{
                .Binding = 2,
                .Resource = albedo->GetImage().View,
            },
            GPU::BindGroupEntry{
                .Binding = 3,
                .Resource = normal->GetImage().View,
            },
            GPU::BindGroupEntry{
                .Binding = 4,
                .Resource = metallic_roughness->GetImage().View,
            },
            GPU::BindGroupEntry{
                .Binding = 5,
                .Resource = ao->GetImage().View,
            },
            GPU::BindGroupEntry{
                .Binding = 6,
                .Resource = emissive->GetImage().View,
            },
            GPU::BindGroupEntry{
                .Binding = 7,
                .Resource = m_LinearSampler,
            },
            GPU::BindGroupEntry{
                .Binding = 8,
                .Resource = m_ShadowSampler,
            },
        };

        GPU::BindGroupSpec global_bg_spec{
            .Label = "Object Bind Group"sv,
            .Entries = bind_group_entries
        };

        auto object_group = Renderer::Device().CreateBindGroup(m_ObjectBindGroupLayout, global_bg_spec);
        m_ObjectGroupsToRelease.push_back(object_group);

        scene_rp.SetVertexBuffer(0, hack.VertexBuffer);
        scene_rp.SetIndexBuffer(hack.IndexBuffer);
        scene_rp.SetBindGroup(object_group, 1);

        scene_rp.DrawIndex({ 0, hack.IndexCount }, { buffer_count_offset, CAST(u32, list.size()) });
        buffer_count_offset += list.size();
    }

    // Draw editor specific stuff only if we are not rendering a game view.
    if (!game_view) {
        Debug::Render(scene_rp);

        scene_rp.SetPipeline(m_GridPipeline);
        scene_rp.Draw({ 0, 6 }, { 0, 1 });
    }

    scene_rp.End();
    scene_rp.Release();
}

void SceneRenderer::CreateSceneRenderTarget(Vector2 const& size)
{
    GPU::TextureSpec spec{
        .Label = "Main Render Target"sv,
        .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3{ size, 1 },
        .Format = GPU::TextureFormat::RGBA8Unorm,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::All,
    };
    m_SceneRenderTarget = Renderer::Device().CreateTexture(spec);
    m_SceneRenderTarget.InitializeView();

    GPU::TextureSpec depth_spec{
        .Label = "Main Render Depth Target"sv,
        .Usage = GPU::TextureUsage::RenderAttachment,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3{ size, 1 },
        .Format = GPU::TextureFormat::Depth24Plus,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::DepthOnly,
    };
    m_SceneRenderDepthTarget = Renderer::Device().CreateTexture(depth_spec);
    m_SceneRenderDepthTarget.InitializeView();
}
