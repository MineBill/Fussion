#include "EditorPCH.h"
#include "SceneRenderer.h"

#include "Fussion/Core/Application.h"
#include "Fussion/GPU/ShaderProcessor.h"
#include "Fussion/Rendering/Renderer.h"
#include "Project/Project.h"

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
    /* #if 0
        {
            const auto scene_rp_spec = RenderPassSpecification{
                .Label = "Scene RenderPass",
                .Attachments = {
                    RenderPassAttachment{
                        .Label = "Color Attachment",
                        .LoadOp = RenderPassAttachmentLoadOp::Clear,
                        .StoreOp = RenderPassAttachmentStoreOp::DontCare,
                        .Format = ImageFormat::B8G8R8A8_UNORM,
                        .Samples = 8,
                        .FinalLayout = ImageLayout::ColorAttachmentOptimal,
                        .ClearColor = { 0.15f, 0.15f, 0.15f, 1.0f },
                    },
                    RenderPassAttachment{
                        .Label = "Color Attachment Resolve Target",
                        .LoadOp = RenderPassAttachmentLoadOp::DontCare,
                        .StoreOp = RenderPassAttachmentStoreOp::Store,
                        .Format = ImageFormat::B8G8R8A8_UNORM,
                        .Samples = 1,
                        .FinalLayout = ImageLayout::ColorAttachmentOptimal,
                    },
                    RenderPassAttachment{
                        .Label = "Depth Attachment",
                        .LoadOp = RenderPassAttachmentLoadOp::Clear,
                        .Format = ImageFormat::D32_SFLOAT,
                        .Samples = 8,
                        .FinalLayout = ImageLayout::DepthStencilAttachmentOptimal,
                        .ClearDepth = 1.f,
                    }
                },
                .SubPasses = {
                    RenderPassSubPass{
                        .ColorAttachments = {
                            {
                                .Attachment = 0,
                                .Layout = ImageLayout::ColorAttachmentOptimal,
                            }
                        },
                        .ResolveAttachments = {
                            {
                                .Attachment = 1,
                                .Layout = ImageLayout::ColorAttachmentOptimal,
                            }
                        },
                        .DepthStencilAttachment = RenderPassAttachmentRef{
                            .Attachment = 2,
                            .Layout = ImageLayout::DepthStencilAttachmentOptimal,
                        }
                    },
                }
            };

            m_SceneRenderPass = Device::Instance()->CreateRenderPass(scene_rp_spec);

            const auto fb_spec = FrameBufferSpecification{
                .Width = 400,
                .Height = 400,
                .Attachments = {
                    FrameBufferAttachmentInfo{
                        .Format = ImageFormat::B8G8R8A8_UNORM,
                        .Usage = ImageUsage::Transient | ImageUsage::ColorAttachment,
                        .Samples = 8,
                    },
                    FrameBufferAttachmentInfo{
                        .Format = ImageFormat::B8G8R8A8_UNORM,
                        .Usage = ImageUsage::Sampled | ImageUsage::ColorAttachment,
                        .Samples = 1,
                    },
                    FrameBufferAttachmentInfo{
                        .Format = ImageFormat::D32_SFLOAT,
                        .Usage = ImageUsage::Transient | ImageUsage::DepthStencilAttachment,
                        .Samples = 8,
                    }
                },
                .Label = "Scene FrameBuffer",
            };
            for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                m_FrameBuffers[i] = Device::Instance()->CreateFrameBuffer(m_SceneRenderPass, fb_spec);
            }
        }

        {
            auto rp_spec = RenderPassSpecification{
                .Label = "Depth RenderPass",
                .Attachments = {
                    RenderPassAttachment{
                        .Label = "Depth Attachment",
                        .LoadOp = RenderPassAttachmentLoadOp::Clear,
                        .StoreOp = RenderPassAttachmentStoreOp::Store,
                        .Format = ImageFormat::D32_SFLOAT,
                        .Samples = 1,
                        .FinalLayout = ImageLayout::DepthStencilReadOnlyOptimal,
                        .ClearDepth = 1.f,
                    },
                },
                .SubPasses = {
                    RenderPassSubPass{
                        .DepthStencilAttachment = RenderPassAttachmentRef{
                            .Attachment = 0,
                            .Layout = ImageLayout::DepthStencilAttachmentOptimal,
                        },
                    },
                },
                .IsShadowMap = true,
            };

            constexpr auto path = "Assets/Shaders/Depth.shader";
            m_DepthPass = Device::Instance()->CreateRenderPass(rp_spec);
            {
                auto data = FileSystem::ReadEntireFile("Assets/Shaders/Depth.shader");
                auto result = ShaderCompiler::Compile(*data);
                auto shader = ShaderAsset::Create(m_DepthPass, result->ShaderStages, result->Metadata);
                m_DepthShader = AssetManager::CreateVirtualAssetRefWithPath<ShaderAsset>(shader, path);
            }

            auto image_spec = ImageSpecification{
                .Label = "Shadow Pass Depth Image",
                .Width = ShadowMapResolution,
                .Height = ShadowMapResolution,
                .Format = rp_spec.Attachments[0].Format,
                .Usage = ImageUsage::DepthStencilAttachment | ImageUsage::Sampled,
                .FinalLayout = ImageLayout::DepthStencilReadOnlyOptimal,
                .SamplerSpec = {},
                // .LayerCount = 1,
                .LayerCount = ShadowCascades,
            };

            m_DepthImage = Device::Instance()->CreateImage(image_spec);

            auto fb_spec = FrameBufferSpecification{
                .Width = ShadowMapResolution,
                .Height = ShadowMapResolution,
                .Label = "Shadow Pass FrameBuffer"
            };

            for (size_t i = 0; i < ShadowCascades; i++) {
                auto iv_spec = ImageViewSpecification{
                    .ViewType = ImageViewType::D2Array,
                    .Format = m_DepthImage->GetSpec().Format,
                    .BaseLayerIndex = CAST(s32, i),
                    .LayerCount = 1,
                };

                auto view = Device::Instance()->CreateImageView(m_DepthImage, iv_spec);
                m_ShadowFrameBuffers[i] = Device::Instance()->CreateFrameBufferFromImageViews(m_DepthPass, { view }, fb_spec);
            }
        }

        {
            auto rp_spec = RenderPassSpecification{
                .Label = "ShadowMap Array Viewer",
                .Attachments = {
                    RenderPassAttachment{
                        .Label = "Depth Debug Display",
                        .LoadOp = RenderPassAttachmentLoadOp::Clear,
                        .StoreOp = RenderPassAttachmentStoreOp::Store,
                        .Format = ImageFormat::B8G8R8A8_UNORM,
                        .Samples = 1,
                        .FinalLayout = ImageLayout::ColorAttachmentOptimal,
                    }
                },
                .SubPasses = {
                    RenderPassSubPass{
                        .ColorAttachments = {
                            {
                                .Attachment = 0,
                                .Layout = ImageLayout::ColorAttachmentOptimal,
                            }
                        },
                    }
                }
            };

            m_ShadowPassDebugRenderPass = Device::Instance()->CreateRenderPass(rp_spec);

            {
                constexpr auto path = "Assets/Shaders/DepthDebugDisplay.shader";
                auto data = FileSystem::ReadEntireFile(path);
                auto result = ShaderCompiler::Compile(*data);
                auto shader = ShaderAsset::Create(m_ShadowPassDebugRenderPass, result->ShaderStages, result->Metadata);
                m_DepthViewerShader = AssetManager::CreateVirtualAssetRefWithPath<ShaderAsset>(shader, path);
            }

            auto fb_spec = FrameBufferSpecification{
                .Width = ShadowMapResolution,
                .Height = ShadowMapResolution,
                .Samples = 1,
                .Attachments = {
                    FrameBufferAttachmentInfo{
                        .Format = ImageFormat::B8G8R8A8_UNORM,
                        .Usage = ImageUsage::Sampled | ImageUsage::ColorAttachment,
                        .Samples = 1,
                    },
                },
                .Label = "ShadowViewer FrameBuffer"
            };
            for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                m_ShadowViewerFrameBuffers[i] = Device::Instance()->CreateFrameBuffer(m_ShadowPassDebugRenderPass, fb_spec);
            }
        }

        {
            const auto rp_spec = RenderPassSpecification{
                .Label = "Object Picking RenderPass",
                .Attachments = {
                    RenderPassAttachment{
                        .Label = "Object Picking ID",
                        .LoadOp = RenderPassAttachmentLoadOp::Clear,
                        .Format = ImageFormat::RED_SIGNED,
                        .Samples = 1,
                        .FinalLayout = ImageLayout::ColorAttachmentOptimal,
                        .ClearColor = { 1, 1, 1, 1 },
                    },
                    RenderPassAttachment{
                        .Label = "Object Picking Depth",
                        .LoadOp = RenderPassAttachmentLoadOp::Clear,
                        .StoreOp = RenderPassAttachmentStoreOp::Store,
                        .Format = ImageFormat::D32_SFLOAT_S8_UINT,
                        .Samples = 1,
                        .FinalLayout = ImageLayout::DepthStencilAttachmentOptimal,
                        .ClearDepth = 1.f,
                    }
                },
                .SubPasses = {
                    RenderPassSubPass{
                        .ColorAttachments = {
                            RenderPassAttachmentRef{
                                .Attachment = 0,
                                .Layout = ImageLayout::ColorAttachmentOptimal,
                            }
                        },
                        .DepthStencilAttachment = RenderPassAttachmentRef{
                            .Attachment = 1,
                            .Layout = ImageLayout::DepthStencilAttachmentOptimal,
                        }
                    },
                }
            };

            constexpr auto path = "Assets/Shaders/ObjectPick.shader";
            m_ObjectPickingRenderPass = Device::Instance()->CreateRenderPass(rp_spec);
            const auto data = FileSystem::ReadEntireFile(path);
            auto result = ShaderCompiler::Compile(*data);
            auto shader = ShaderAsset::Create(m_ObjectPickingRenderPass, result->ShaderStages, result->Metadata);
            m_ObjectPickingShader = AssetManager::CreateVirtualAssetRefWithPath<ShaderAsset>(shader, path);

            auto fb_spec = FrameBufferSpecification{
                .Width = 100,
                .Height = 100,
                .Attachments = {
                    FrameBufferAttachmentInfo{
                        .Format = ImageFormat::RED_SIGNED,
                        .Usage = ImageUsage::ColorAttachment | ImageUsage::Sampled | ImageUsage::TransferSrc,
                        .Samples = 1
                    },
                    FrameBufferAttachmentInfo{
                        .Format = ImageFormat::D32_SFLOAT_S8_UINT,
                        .Usage = ImageUsage::DepthStencilAttachment | ImageUsage::Transient,
                        .Samples = 1
                    }
                },
                .Label = "Object picking FrameBuffer"
            };

            m_ObjectPickingFrameBuffer = Device::Instance()->CreateFrameBuffer(m_ObjectPickingRenderPass, fb_spec);
        }

        {
            constexpr auto path = "Assets/Shaders/PbrObject.shader";
            auto const data = FileSystem::ReadEntireFile(path);
            auto result = ShaderCompiler::Compile(*data);
            auto shader = ShaderAsset::Create(m_SceneRenderPass, result->ShaderStages, result->Metadata);

            m_PbrShader = AssetManager::CreateVirtualAssetRefWithPath<ShaderAsset>(shader, path);
        }

        {
            constexpr auto path = "Assets/Shaders/Editor/Grid.shader";
            auto data = FileSystem::ReadEntireFile(path);
            auto result = ShaderCompiler::Compile(*data);
            auto shader = ShaderAsset::Create(m_SceneRenderPass, result->ShaderStages, result->Metadata);

            m_GridShader = AssetManager::CreateVirtualAssetRefWithPath<ShaderAsset>(shader, path);
        }

        {
            constexpr auto path = "Assets/Shaders/Sky.shader";
            auto data = FileSystem::ReadEntireFile(path);
            auto result = ShaderCompiler::Compile(*data);
            auto shader = ShaderAsset::Create(m_SceneRenderPass, result->ShaderStages, result->Metadata);

            m_SkyShader = AssetManager::CreateVirtualAssetRefWithPath<ShaderAsset>(shader, path);
        }

        SceneViewData = UniformBuffer<ViewData>::Create("View Data");
        SceneDebugOptions = UniformBuffer<DebugOptions>::Create("Debug Options");
        SceneGlobalData = UniformBuffer<GlobalData>::Create("Global Data");
        SceneSceneData = UniformBuffer<SceneData>::Create("Scene Data");
        SceneLightData = UniformBuffer<LightData>::Create("Light Data");

        auto pool_spec = ResourcePoolSpecification::Default(100);
        m_ResourcePool.fill(Device::Instance()->CreateResourcePool(pool_spec));

        auto current_frame = Renderer::GetInstance()->GetSwapchain()->GetCurrentFrame();
        {
            std::vector resource_usages = {
                ResourceUsage{
                    .Label = "ViewData",
                    .Type = ResourceType::UniformBuffer,
                    .Stages = ShaderType::Vertex | ShaderType::Fragment,
                    .Binding = 0,
                },
                ResourceUsage{
                    .Label = "DebugOptions",
                    .Type = ResourceType::UniformBuffer,
                    .Stages = ShaderType::Vertex | ShaderType::Fragment,
                    .Binding = 1,
                },
                ResourceUsage{
                    .Label = "GlobalData",
                    .Type = ResourceType::UniformBuffer,
                    .Stages = ShaderType::Vertex | ShaderType::Fragment,
                    .Binding = 2,
                }
            };
            auto layout = Device::Instance()->CreateResourceLayout(resource_usages);

            for (int i = 0; i < ::RHI::MAX_FRAMES_IN_FLIGHT; ++i) {
                auto result = m_ResourcePool[current_frame]->Allocate(layout);
                if (result.IsError()) {
                    LOG_ERRORF("Error while allocating resource: {}", magic_enum::enum_name(result.Error()));
                    return;
                }

                m_GlobalResource[i] = result.Value();
            }
        }

        {
            std::vector resource_usages = {
                ResourceUsage{
                    .Label = "SceneData",
                    .Type = ResourceType::UniformBuffer,
                    .Stages = ShaderType::Vertex | ShaderType::Fragment,
                    .Binding = 0,
                },
                ResourceUsage{
                    .Label = "LightData",
                    .Type = ResourceType::UniformBuffer,
                    .Stages = ShaderType::Vertex | ShaderType::Fragment,
                    .Binding = 1,
                },
                ResourceUsage{
                    .Label = "ShadowMap",
                    .Type = ResourceType::CombinedImageSampler,
                    .Count = 1,
                    .Stages = ShaderType::Fragment,
                    .Binding = 2
                },
            };
            auto layout = Device::Instance()->CreateResourceLayout(resource_usages);

            for (int i = 0; i < ::RHI::MAX_FRAMES_IN_FLIGHT; ++i) {
                auto result = m_ResourcePool[current_frame]->Allocate(layout);
                if (result.IsError()) {
                    LOG_ERRORF("Error while allocating resource: {}", magic_enum::enum_name(result.Error()));
                    return;
                }
                m_SceneResource[i] = result.Value();
            }
        }

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            BufferSpecification spec{
                .Label = fmt::format("Instance Buffer {}", i),
                .Usage = BufferUsage::Storage,
                .Size = sizeof(Mat4) * 2 * 1'000,
                .Mapped = true,
            };
            m_InstanceBuffers[i] = Device::Instance()->CreateBuffer(spec);
            m_PBRInstanceBuffers[i] = Device::Instance()->CreateBuffer(spec);
        }

        m_FrameAllocator.Init(MAX_FRAMES_IN_FLIGHT, "Object Frame Allocator");
        m_ObjectDepthResourceUsages = {
            ResourceUsage{
                .Label = "Instance Data",
                .Type = ResourceType::StorageBuffer,
                .Count = 1,
                .Stages = ShaderType::Vertex,
                .Binding = 0,
            },
        };

        //       this from the shader.
        m_PBRResourceUsages = {
            ResourceUsage{
                .Label = "Material",
                .Type = ResourceType::UniformBuffer,
                .Count = 1,
                .Stages = ShaderType::Vertex | ShaderType::Fragment,
                .Binding = 0,
            },
            ResourceUsage{
                .Label = "Albedo Map",
                .Type = ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = ShaderType::Fragment,
                .Binding = 1,
            },
            ResourceUsage{
                .Label = "Normal Map",
                .Type = ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = ShaderType::Fragment,
                .Binding = 2,
            },
            ResourceUsage{
                .Label = "AmbientOcclusion Map",
                .Type = ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = ShaderType::Fragment,
                .Binding = 3,
            },
            ResourceUsage{
                .Label = "MetallicRoughness Map",
                .Type = ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = ShaderType::Fragment,
                .Binding = 4,
            },
            ResourceUsage{
                .Label = "Emissive Map",
                .Type = ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = ShaderType::Fragment,
                .Binding = 5,
            },
            ResourceUsage{
                .Label = "Instance Data",
                .Type = ResourceType::StorageBuffer,
                .Count = 1,
                .Stages = ShaderType::Vertex,
                .Binding = 6,
            },
        };

        Debug::Initialize(m_SceneRenderPass);
    #endif */
    auto window_size = Application::Instance()->GetWindow().GetSize();

    CreateSceneRenderTarget(window_size);

    SceneViewData = UniformBuffer<ViewData>::Create(Renderer::Device(), std::string_view{ "View Data" });
    SceneLightData = UniformBuffer<LightData>::Create(Renderer::Device(), std::string_view{ "Light Data" });

    ///////////////////////
    /// BIND GROUP CREATION
    ///////////////////////

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
        };

        GPU::BindGroupLayoutSpec spec{
            .Label = "Global BGL",
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
            }
        };

        GPU::BindGroupSpec global_bg_spec{
            .Label = "Global Bind Group",
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

        };

        GPU::BindGroupLayoutSpec spec{
            .Label = "Object BGL",
            .Entries = entries,
        };

        m_ObjectBindGroupLayout = Renderer::Device().CreateBindGroupLayout(spec);

        // std::array bind_group_entries{
        //     GPU::BindGroupEntry{
        //         .Binding = 0,
        //         .Resource = GPU::BufferBinding{
        //             .Buffer = SceneViewData.GetBuffer(),
        //             .Offset = 0,
        //             .Size = SceneViewData.Size(),
        //         }
        //     }
        // };
        //
        // GPU::BindGroupSpec global_bg_spec{
        //     .Label = "Object Bind Group",
        //     .Entries = bind_group_entries
        // };
        //
        // auto object_group = Renderer::Device().CreateBindGroup(m_ObjectBindGroupLayout, global_bg_spec);
    }

    ////////////////////////
    /// RENDER PASS CREATION
    ////////////////////////

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/simple.wgsl").Value();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "Simple WGSL Shader",
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
            .Label = None(),
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        GPU::RenderPipelineSpec rp_spec{
            .Label = "Simple RP",
            .Layout = layout,
            .Vertex = {},
            .Primitive = GPU::PrimitiveState::Default(),
            .DepthStencil = None(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = {
                .Targets = {
                    GPU::ColorTargetState{
                        .Format = GPU::TextureFormat::RGBA8Unorm,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    }
                }
            },
        };

        m_SimpleRp = Renderer::Device().CreateRenderPipeline(shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/WGSL/PBR.wgsl").Value();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "PBR Shader",
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
            .Label = None(),
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
            .Label = "PBR Render Pass",
            .Layout = layout,
            .Vertex = {
                .AttributeLayouts = { attribute_layout }
            },
            .Primitive = {
                .Topology = GPU::PrimitiveTopology::TriangleList,
                .StripIndexFormat = None(),
                .FrontFace = GPU::FrontFace::Ccw,
                .Cull = GPU::Face::None,
            },
            .DepthStencil = GPU::DepthStencilState::Default(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = {
                .Targets = {
                    GPU::ColorTargetState{
                        .Format = GPU::TextureFormat::RGBA8Unorm,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    },
                }
            },
        };

        m_PbrRp = Renderer::Device().CreateRenderPipeline(shader, rp_spec);
    }

    {
        auto shader_src = GPU::ShaderProcessor::ProcessFile("Assets/Shaders/Editor/Grid.wgsl").Value();

        GPU::ShaderModuleSpec shader_spec{
            .Label = "Grid Shader",
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
            .Label = None(),
            .BindGroupLayouts = bind_group_layouts
        };
        auto layout = Renderer::Device().CreatePipelineLayout(pl_spec);

        GPU::RenderPipelineSpec rp_spec{
            .Label = "Grid RP",
            .Layout = layout,
            .Vertex = {},
            .Primitive = GPU::PrimitiveState::Default(),
            .DepthStencil = GPU::DepthStencilState::Default(),
            .MultiSample = GPU::MultiSampleState::Default(),
            .Fragment = {
                .Targets = {
                    GPU::ColorTargetState{
                        .Format = GPU::TextureFormat::RGBA8Unorm,
                        .Blend = GPU::BlendState::Default(),
                        .WriteMask = GPU::ColorWrite::All,
                    }
                }
            },
        };

        m_GridRp = Renderer::Device().CreateRenderPipeline(shader, rp_spec);
    }

    GPU::BufferSpec ibs{
        .Label = "PBR Instance Buffer",
        .Usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        .Size = sizeof(Mat4) * 1'000,
        .Mapped = false,
    };
    m_PbrInstanceBuffer = Renderer::Device().CreateBuffer(ibs);

    m_PbrInstanceStagingBuffer.reserve(sizeof(Mat4) * 1'000);

    GPU::SamplerSpec bilinear_sampler_spec{
        .Label = "Bilinear Sampler",
        .AddressModeU = GPU::AddressMode::Repeat,
        .AddressModeV = GPU::AddressMode::Repeat,
        .AddressModeW = GPU::AddressMode::Repeat,
        .MagFilter = GPU::FilterMode::Linear,
        .MinFilter = GPU::FilterMode::Linear,
        .MipMapFilter = GPU::FilterMode::Linear,
        .LodMinClamp = 1,
        .LodMaxClamp = 1,
        .AnisotropyClamp = 1
    };

    m_LinearSampler = Renderer::Device().CreateSampler(bilinear_sampler_spec);
}

void SceneRenderer::Resize(Vector2 const& new_size)
{
    ZoneScoped;
    m_RenderArea = new_size;

    m_SceneRenderTarget.Release();
    m_SceneRenderDepthTarget.Release();
    CreateSceneRenderTarget(new_size);
    /* #if 0
        ZoneScoped;
        Device::Instance()->WaitIdle();
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            m_FrameBuffers[i]->Resize(new_size);
        }
        m_ObjectPickingFrameBuffer->Resize(new_size);
    #endif */
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

    {
        std::array color_attachments{
            GPU::RenderPassColorAttachment{
                .View = m_SceneRenderTarget.View,
                .LoadOp = GPU::LoadOp::Clear,
                .StoreOp = GPU::StoreOp::Store,
                .ClearColor = Color::Black,
            },
        };

        GPU::RenderPassSpec scene_rp_spec{
            .Label = "Scene Render Pass",
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

        scene_rp.SetPipeline(m_PbrRp);
        scene_rp.SetBindGroup(m_GlobalBindGroup, 0);

        u32 buffer_offset = 0;
        for (auto const& [buffer, list] : m_RenderContext.MeshRenderLists) {
            ZoneScopedN("PBR Command Buffer Dispatch");
            (void)buffer;

            auto const& hack = m_RenderContext.RenderObjects[list[0]];

#if 1
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

            // cmd->BindImage(albedo->GetImage(), resource, 1);
            // cmd->BindImage(normal->GetImage(), resource, 2);
            //
            // cmd->BindImage(ao->GetImage(), resource, 3);
            // cmd->BindImage(metallic_roughness->GetImage(), resource, 4);
            // cmd->BindImage(emissive->GetImage(), resource, 5);
#endif

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
                    .Binding = 1,
                    .Resource = GPU::BufferBinding{
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
            };

            GPU::BindGroupSpec global_bg_spec{
                .Label = "Object Bind Group",
                .Entries = bind_group_entries
            };

            auto object_group = Renderer::Device().CreateBindGroup(m_ObjectBindGroupLayout, global_bg_spec);
            m_DeadObjectGroups.push_back(object_group);
            // defer(object_group.Release());

            auto data = TRANSMUTE(InstanceData*, m_PbrInstanceStagingBuffer.data()) + buffer_offset;
            int j = 0;
            for (auto index : list) {
                auto& obj = m_RenderContext.RenderObjects[index];
                data[j++].Model = obj.WorldMatrix;
            }

            auto byte_offset = buffer_offset * sizeof(InstanceData);
            Renderer::Device().WriteBuffer(
                m_PbrInstanceBuffer, byte_offset,
                m_PbrInstanceStagingBuffer.data() + byte_offset, list.size() * sizeof(InstanceData));

            scene_rp.SetVertexBuffer(0, hack.VertexBuffer);
            scene_rp.SetIndexBuffer(hack.IndexBuffer);
            scene_rp.SetBindGroup(object_group, 1);

            scene_rp.DrawIndex({ 0, hack.IndexCount }, { buffer_offset, CAST(u32, list.size()) });
            buffer_offset += list.size();
        }

        // Draw editor specific stuff only if we are not rendering a game view.
        if (!game_view) {
            scene_rp.SetPipeline(m_GridRp);
            scene_rp.SetBindGroup(m_GlobalBindGroup, 0);

            scene_rp.Draw({ 0, 6 }, { 0, 1 });
        }

        scene_rp.End();
        scene_rp.Release();
    }

    for (auto& group : m_DeadObjectGroups) {
        group.Release();
    }
    m_DeadObjectGroups.clear();

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

void SceneRenderer::CreateSceneRenderTarget(Vector2 const& size)
{
    GPU::TextureSpec spec{
        .Label = "Main Render Target",
        .Usage = GPU::TextureUsage::RenderAttachment | GPU::TextureUsage::TextureBinding,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3{ size, 1 },
        .Format = GPU::TextureFormat::RGBA8Unorm,
        .MipLevelCount = 1,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::All,
    };
    m_SceneRenderTarget = Renderer::Device().CreateTexture(spec);
    m_SceneRenderTarget.InitializeView();

    GPU::TextureSpec depth_spec{
        .Label = "Main Render Depth Target",
        .Usage = GPU::TextureUsage::RenderAttachment,
        .Dimension = GPU::TextureDimension::D2,
        .Size = Vector3{ size, 1 },
        .Format = GPU::TextureFormat::Depth24Plus,
        .MipLevelCount = 1,
        .SampleCount = 1,
        .Aspect = GPU::TextureAspect::DepthOnly,
    };
    m_SceneRenderDepthTarget = Renderer::Device().CreateTexture(depth_spec);
    m_SceneRenderDepthTarget.InitializeView();
}
