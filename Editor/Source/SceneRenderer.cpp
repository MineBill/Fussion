#include "EditorPCH.h"
#include "SceneRenderer.h"

#include "Fussion/Input/Input.h"
#include "Project/Project.h"

#include <Fussion/Assets/ShaderAsset.h>
#include <Fussion/Assets/AssetManager.h>
#include <Fussion/Core/Time.h>
#include <Fussion/Debug/Debug.h>
#include <Fussion/OS/FileSystem.h>
#include <Fussion/RHI/Device.h>
#include <Fussion/RHI/ShaderCompiler.h>
#include <Fussion/Util/TextureImporter.h>

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#undef far
#undef near
#undef min
#undef max

using namespace Fussion;
using namespace Fussion::RHI;

void SceneRenderer::Init()
{
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
            }
        };
        m_FrameBuffer = Device::Instance()->CreateFrameBuffer(m_SceneRenderPass, fb_spec);
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
            }
        };
        m_ShadowViewerFrameBuffer = Device::Instance()->CreateFrameBuffer(m_ShadowPassDebugRenderPass, fb_spec);
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
            }
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
    m_ResourcePool = Device::Instance()->CreateResourcePool(pool_spec);

    {
        std::vector resource_usages = {
            ResourceUsage{
                .Label = "ViewData",
                .Type = ResourceType::UniformBuffer,
                .Stages = ShaderType::Vertex | ShaderType::Fragment,
            },
            ResourceUsage{
                .Label = "DebugOptions",
                .Type = ResourceType::UniformBuffer,
                .Stages = ShaderType::Vertex | ShaderType::Fragment,
            },
            ResourceUsage{
                .Label = "GlobalData",
                .Type = ResourceType::UniformBuffer,
                .Stages = ShaderType::Vertex | ShaderType::Fragment,
            }
        };
        auto layout = Device::Instance()->CreateResourceLayout(resource_usages);
        auto result = m_ResourcePool->Allocate(layout);
        if (result.IsError()) {
            LOG_ERRORF("Error while allocating resource: {}", magic_enum::enum_name(result.Error()));
            return;
        }

        m_GlobalResource = result.Value();
    }

    {
        std::vector resource_usages = {
            ResourceUsage{
                .Label = "SceneData",
                .Type = ResourceType::UniformBuffer,
                .Stages = ShaderType::Vertex | ShaderType::Fragment,
            },
            ResourceUsage{
                .Label = "LightData",
                .Type = ResourceType::UniformBuffer,
                .Stages = ShaderType::Vertex | ShaderType::Fragment,
            },
            ResourceUsage{
                .Label = "ShadowMap",
                .Type = ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = ShaderType::Fragment,
            },
        };
        auto layout = Device::Instance()->CreateResourceLayout(resource_usages);
        auto result = m_ResourcePool->Allocate(layout);
        if (result.IsError()) {
            LOG_ERRORF("Error while allocating resource: {}", magic_enum::enum_name(result.Error()));
            return;
        }

        m_SceneResource = result.Value();
    }

    Debug::Initialize(m_SceneRenderPass);
    m_TestTexture = TextureImporter::LoadTextureFromFile(std::filesystem::current_path() / "Assets" / "coords.png");
}

void SceneRenderer::Resize(Vector2 const& new_size)
{
    ZoneScoped;
    Device::Instance()->WaitIdle();
    m_RenderArea = new_size;
    m_FrameBuffer->Resize(new_size);
    m_ObjectPickingFrameBuffer->Resize(new_size);
}

f32 GetSplitDepth(s32 current_split, s32 max_splits, f32 near, f32 far, f32 l = 1.0f)
{
    auto split_ratio = CAST(f32, current_split) / CAST(f32, max_splits);
    auto log = near * Math::Pow(far / near, split_ratio);
    auto uniform = near + (far - near) * split_ratio;
    auto d = l * (log - uniform) + uniform;
    return (d - near) / (far - near);
}

void SceneRenderer::Render(Ref<CommandBuffer> const& cmd, RenderPacket const& packet, bool game_view)
{
    ZoneScoped;
    using namespace Fussion;

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

    cmd->BindUniformBuffer(SceneViewData.GetBuffer(), m_GlobalResource, 0);
    cmd->BindUniformBuffer(SceneDebugOptions.GetBuffer(), m_GlobalResource, 1);
    cmd->BindUniformBuffer(SceneGlobalData.GetBuffer(), m_GlobalResource, 2);

    cmd->BindUniformBuffer(SceneSceneData.GetBuffer(), m_SceneResource, 0);
    cmd->BindUniformBuffer(SceneLightData.GetBuffer(), m_SceneResource, 1);
    cmd->BindImage(m_DepthImage, m_SceneResource, 2);

    {
        ZoneScopedN("Light Collection");
        m_RenderContext.RenderFlags = RenderState::LightCollection;
        if (packet.Scene) {
            packet.Scene->ForEachEntity([&](Entity* entity) {
                entity->OnDraw(m_RenderContext);
            });
        }
    }

    if (!m_RenderContext.DirectionalLights.empty()) {
        SceneLightData.Data.DirectionalLight = m_RenderContext.DirectionalLights[0].ShaderData;
    }
    SceneLightData.Flush();

    {
        ZoneScopedN("Depth Pass");
        m_RenderContext.RenderFlags = RenderState::Depth;

        for (auto& light : m_RenderContext.DirectionalLights) {
            for (auto i = 0; i < ShadowCascades; i++) {
                SceneLightData.Data.ShadowSplitDistances[i] = GetSplitDepth(i + 1, ShadowCascades, packet.Camera.Near, packet.Camera.Far, light.Split);
            }

            cmd->SetViewport({ ShadowMapResolution, -ShadowMapResolution });
            cmd->SetScissor(Vector4(0, 0, ShadowMapResolution, ShadowMapResolution));

            f32 last_split{ 0 };
            for (auto i = 0; i < ShadowCascades; i++) {

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

                auto split = SceneLightData.Data.ShadowSplitDistances[i];
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
                SceneLightData.Data.DirectionalLight.LightSpaceMatrix[i] = light.ShaderData.LightSpaceMatrix[i];
                if (packet.Scene) {
                    m_RenderContext.CurrentShader = depth_shader;
                    m_RenderContext.CurrentLightSpace = light.ShaderData.LightSpaceMatrix[i];
                    packet.Scene->ForEachEntity([&](Entity* entity) {
                        ZoneScopedN("Entity Draw");
                        entity->OnDraw(m_RenderContext);
                    });
                }
                cmd->EndRenderPass(m_DepthPass);
                last_split = split;
            }
            SceneLightData.Flush();

            cmd->BeginRenderPass(m_ShadowPassDebugRenderPass, m_ShadowViewerFrameBuffer);
            {
                ZoneScopedN("ShadowMap Debug Viewer");

                auto shader = m_DepthViewerShader.Get()->GetShader();
                cmd->UseShader(shader);
                cmd->BindResource(m_SceneResource, shader, 1);
                struct {
                    s32 CascadeIndex;
                } pc;
                pc.CascadeIndex = RenderDebugOptions.CascadeIndex;;
                cmd->PushConstants(shader, &pc, sizeof(pc));
                cmd->Draw(6, 1);
            }
            cmd->EndRenderPass(m_ShadowPassDebugRenderPass);
        }
    }

    cmd->SetViewport({ m_RenderArea.X, -m_RenderArea.Y });
    cmd->SetScissor(Vector4(0, 0, m_RenderArea.X, m_RenderArea.Y));

    cmd->BeginRenderPass(m_ObjectPickingRenderPass, m_ObjectPickingFrameBuffer);
    {
        ZoneScopedN("Object Picking Render Pass");
        auto object_pick_shader = m_ObjectPickingShader.Get()->GetShader();

        m_RenderContext.RenderFlags = RenderState::ObjectPicking;
        m_RenderContext.CurrentPass = m_ObjectPickingRenderPass;
        m_RenderContext.CurrentShader = object_pick_shader;

        cmd->UseShader(object_pick_shader);
        cmd->BindResource(m_GlobalResource, object_pick_shader, 0);

        if (packet.Scene) {
            packet.Scene->ForEachEntity([&](Entity* entity) {
                ZoneScopedN("Object Picking Draw");
                entity->OnDraw(m_RenderContext);
            });
        }
    }
    cmd->EndRenderPass(m_ObjectPickingRenderPass);

    cmd->BeginRenderPass(m_SceneRenderPass, m_FrameBuffer);
    {
        m_RenderContext.CurrentPass = m_SceneRenderPass;

        ZoneScopedN("Scene Render Pass");
        cmd->SetViewport({ m_RenderArea.X, -m_RenderArea.Y });
        cmd->SetScissor(Vector4(0, 0, m_RenderArea.X, m_RenderArea.Y));

        {
            ZoneScopedN("Skybox");

            auto sky_shader = m_SkyShader.Get()->GetShader();
            cmd->UseShader(sky_shader);
            cmd->BindResource(m_GlobalResource, sky_shader, 0);
            cmd->BindResource(m_SceneResource, sky_shader, 1);
            cmd->Draw(4, 1);
        }

        {
            ZoneScopedN("PBR");
            m_RenderContext.RenderFlags = RenderState::Mesh;
            auto pbr_shader = m_PbrShader.Get()->GetShader();
            cmd->UseShader(pbr_shader);
            cmd->BindResource(m_GlobalResource, pbr_shader, 0);
            cmd->BindResource(m_SceneResource, pbr_shader, 1);

            m_RenderContext.CurrentShader = pbr_shader;
            if (packet.Scene) {
                packet.Scene->ForEachEntity([&](Entity* entity) {
                    ZoneScopedN("Entity Draw");
                    entity->OnDraw(m_RenderContext);
                });
            }
        }

        if (!game_view) {
            {
                ZoneScopedN("Debug Draw");
                Debug::Render(cmd, m_GlobalResource);
            }

            {
                ZoneScopedN("Editor Grid");
                auto grid_shader = m_GridShader.Get()->GetShader();
                m_RenderContext.CurrentShader = grid_shader;
                cmd->UseShader(grid_shader);
                cmd->BindResource(m_GlobalResource, grid_shader, 0);
                // cmd->BindResource(m_SceneResource, m_GridShader, 1);
                cmd->Draw(6, 1);
            }
        }
        Debug::Reset();

    }
    cmd->EndRenderPass(m_SceneRenderPass);
}
