#include "SceneRenderer.h"

#include "Assets/Importers/TextureImporter.h"
#include "Fussion/Debug/Debug.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#undef far
#undef near
#undef min
#undef max

#include "Fussion/Assets/ShaderAsset.h"
#include "Fussion/Assets/AssetManager.h"
#include "Fussion/OS/FileSystem.h"
#include "Fussion/RHI/Device.h"
#include "Fussion/RHI/Renderer.h"
#include "Fussion/RHI/ShaderCompiler.h"
#include "Project/Project.h"

using namespace Fussion;
using namespace Fussion::RHI;

constexpr auto ShadowCascades = 4;
constexpr auto ShadowMapResolution = 4096;

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
        const auto rp_spec = RenderPassSpecification{
            .Label = "Depth RenderPass",
            .Attachments = {
                RenderPassAttachment{
                    .Label = "Depth Attachment",
                    .LoadOp = RenderPassAttachmentLoadOp::Clear,
                    .StoreOp = RenderPassAttachmentStoreOp::Store,
                    .Format = ImageFormat::D32_SFLOAT,
                    .Samples = 1,
                    .FinalLayout = ImageLayout::DepthStencilAttachmentOptimal,
                    .ClearDepth = 1.f,
                }
            },
            .SubPasses = {
                RenderPassSubPass{
                    .DepthStencilAttachment = RenderPassAttachmentRef{
                        .Attachment = 0,
                        .Layout = ImageLayout::DepthStencilAttachmentOptimal,
                    }
                },
            }
        };

        m_DepthPass = Device::Instance()->CreateRenderPass(rp_spec);
        const auto data = FileSystem::ReadEntireFile("Assets/Shaders/Depth.shader");
        auto [stages, metadata] = ShaderCompiler::Compile(*data);
        m_DepthShader = Device::Instance()->CreateShader(m_DepthPass, stages, metadata);

        auto image_spec = RHI::ImageSpecification{
            .Label = "Shadow Pass Depth Image",
            .Width = ShadowMapResolution,
            .Height = ShadowMapResolution,
            .Format = rp_spec.Attachments[0].Format,
            .Usage = ImageUsage::DepthStencilAttachment | ImageUsage::Sampled,
            .Layout = ImageLayout::Undefined,
            .FinalLayout = ImageLayout::DepthStencilReadOnlyOptimal,
            .SamplerSpec = {},
            .LayerCount = 4,
        };

        m_DepthImage = Device::Instance()->CreateImage(image_spec);

        for (auto i = 0; i < m_ShadowFrameBuffers.size(); i++) {
            auto spec = RHI::FrameBufferSpecification{
                .Width = ShadowMapResolution,
                .Height = ShadowMapResolution,
            };

            auto iv_spec = RHI::ImageViewSpecification{
                .ViewType = ImageViewType::D2Array,
                .Format = m_DepthImage->GetSpec().Format,
                .BaseLayerIndex = i,
                .LayerCount = 1,
            };

            auto view = Device::Instance()->CreateImageView(m_DepthImage, iv_spec);

            m_ShadowFrameBuffers[i] = Device::Instance()->CreateFrameBufferFromImageViews(m_DepthPass, { view }, spec);
        }
    }

    {
        // auto shader = AssetManager::GetAsset<ShaderAsset>("Assets/Shaders/SimplePBR.shader");
        // shader->GetShader();

        const auto data = FileSystem::ReadEntireFile("Assets/Shaders/SimplePBR.shader");
        auto [stages, metadata] = ShaderCompiler::Compile(*data);
        metadata.Samples = 8;
        m_PbrShader = Device::Instance()->CreateShader(m_SceneRenderPass, stages, metadata);
    }

    {
        const auto data = FileSystem::ReadEntireFile("Assets/Shaders/Editor/Grid.shader");
        auto [stages, metadata] = ShaderCompiler::Compile(*data);
        metadata.UseBlending = true;
        metadata.Samples = 8;
        m_GridShader = Device::Instance()->CreateShader(m_SceneRenderPass, stages, metadata);
    }

    m_ViewData = UniformBuffer<ViewData>::Create("View Data");
    m_DebugOptions = UniformBuffer<DebugOptions>::Create("Debug Options");
    m_GlobalData = UniformBuffer<GlobalData>::Create("Global Data");

    const auto pool_spec = ResourcePoolSpecification::Default(100);
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

        m_GlobalResource = result.TakeValue();
    }

    {
        std::vector resource_usages = {
            ResourceUsage{
                .Label = "SceneData",
                .Type = ResourceType::UniformBuffer,
                .Stages = ShaderType::Vertex | ShaderType::Fragment,
            },
        };
        auto layout = Device::Instance()->CreateResourceLayout(resource_usages);
        auto result = m_ResourcePool->Allocate(layout);
        if (result.IsError()) {
            LOG_ERRORF("Error while allocating resource: {}", magic_enum::enum_name(result.Error()));
            return;
        }

        m_SceneResource = result.TakeValue();
    }

    Debug::Initialize(m_SceneRenderPass);
    m_TestTexture = TextureImporter::LoadTextureFromFile(std::filesystem::current_path() / "Assets" / "coords.png");
}

void SceneRenderer::Resize(const Vector2 new_size)
{
    ZoneScoped;
    Device::Instance()->WaitIdle();
    m_RenderArea = new_size;
    m_FrameBuffer->Resize(new_size);
}

f32 GetSplitDepth(s32 current_split, s32 max_splits, f32 near, f32 far, f32 l = 1.0f)
{
    auto split_ratio = CAST(f32, current_split) / CAST(f32, max_splits);
    auto log = near * Math::Pow(far / near, split_ratio);
    auto uniform = near + (far - near) * split_ratio;
    auto d = l * (log - uniform) + uniform;
    return (d - near) / (far - near);
}

auto GetFrustumCornersWorldSpace(Mat4 const& proj, Mat4 const& view) -> std::array<Vector4, 8>
{
    auto inv = glm::inverse(proj * view);
    std::array<Vector4, 8> corners;

    auto i = 0;
    for (auto x = 0; x < 2; x++) {
        for (auto y = 0; y < 2; y++) {
            for (auto z = 0; z < 2; z++) {
                auto pt = inv * Vector4{
                    2 * CAST(f32, x) - 1,
                    2 * CAST(f32, y) - 1,
                    2 * CAST(f32, z) - 1,
                    1.0 };

                corners[i] = pt / pt.w;

                i += 1;
            }
        }
    }
    return corners;
}

void SceneRenderer::Render(Ref<CommandBuffer> const& cmd, RenderPacket const& packet)
{
    ZoneScoped;
    using namespace Fussion;

    m_ViewData.Data.Perspective = packet.Camera.Perspective;
    m_ViewData.Data.View = packet.Camera.View;
    m_ViewData.Flush();

    m_RenderContext.Cmd = cmd;
    m_RenderContext.DirectionalLights.clear();
    m_RenderContext.PointLights.clear();

    {
        ZoneScopedN("Light Collection");
        m_RenderContext.RenderFlags = RenderState::LightCollection;
        if (packet.Scene) {
            packet.Scene->ForEachEntity([&](Entity* entity) {
                entity->OnDraw(m_RenderContext);
            });
        }
    }

    {
        ZoneScopedN("Depth Pass");
        m_RenderContext.RenderFlags = RenderState::Depth;

        std::array<f32, ShadowCascades> distances{};

        for (auto i = 0; i < ShadowCascades; i++) {
            for (auto& light : m_RenderContext.DirectionalLights) {
                (void)light;
                distances[i] = GetSplitDepth(i + 1, ShadowCascades, packet.Camera.Near, packet.Camera.Far, 1.0f);
            }
        }

        for (auto i = 0; i < ShadowCascades; i++) {
            cmd->BeginRenderPass(m_DepthPass, m_ShadowFrameBuffers[i]);
            cmd->SetViewport({ ShadowMapResolution, -ShadowMapResolution });
            cmd->SetScissor(Vector4(0, 0, ShadowMapResolution, ShadowMapResolution));

            cmd->UseShader(m_DepthShader);
            for (auto& light : m_RenderContext.DirectionalLights) {
                auto _proj = glm::perspective(
                    glm::radians(50.0f),
                    m_RenderArea.Aspect(),
                    packet.Camera.Near,
                    packet.Camera.Far * distances[i]);
                auto corners = GetFrustumCornersWorldSpace(_proj, packet.Camera.View);

                Vector3 center;
                for (auto const& corner : corners) {
                    center += corner;
                }
                center /= 8.0f;

                auto view = glm::lookAt(glm::vec3(center + Vector3(light.Direction)), glm::vec3(center), glm::vec3(Vector3::Up));

                auto maxf = std::numeric_limits<f32>::min();
                auto minf = std::numeric_limits<f32>::max();

                Vector3 min{ minf, minf, minf };
                Vector3 max{ maxf, maxf, maxf };

                for (auto const& corner : corners) {
                    auto hm = glm::vec3(view * corner);
                    if (hm.x < min.X)
                        min.X = hm.x;
                    if (hm.y < min.Y)
                        min.Y = hm.y;
                    if (hm.z < min.Z)
                        min.Z = hm.z;
                    if (hm.x > max.X)
                        max.X = hm.x;
                    if (hm.y > max.Y)
                        max.Y = hm.y;
                    if (hm.z > max.Z)
                        max.Z = hm.z;
                }

                auto proj = glm::ortho(min.X, max.X, min.Y, max.Y, min.Z * 10.0f, max.Z / 10.0f);

                light.LightSpaceMatrix[i] = proj * view;
                if (packet.Scene) {
                    m_RenderContext.CurrentShader = m_DepthShader;
                    m_RenderContext.CurrentLightSpace = light.LightSpaceMatrix[i];
                    packet.Scene->ForEachEntity([&](Entity* entity) {
                        ZoneScopedN("Entity Draw");
                        entity->OnDraw(m_RenderContext);
                    });
                }
            }

            cmd->EndRenderPass(m_DepthPass);
        }
    }

    cmd->BeginRenderPass(m_SceneRenderPass, m_FrameBuffer);
    {
        m_RenderContext.CurrentPass = m_SceneRenderPass;

        ZoneScopedN("Scene Render Pass");
        cmd->SetViewport({ m_RenderArea.X, -m_RenderArea.Y });
        cmd->SetScissor(Vector4(0, 0, m_RenderArea.X, m_RenderArea.Y));

        {
            ZoneScopedN("PBR");
            m_RenderContext.RenderFlags = RenderState::Mesh;
            cmd->UseShader(m_PbrShader);
            cmd->BindResource(m_GlobalResource, m_PbrShader, 0);
            cmd->BindUniformBuffer(m_ViewData.GetBuffer(), m_GlobalResource, 0);

            m_RenderContext.CurrentShader = m_PbrShader;
            if (packet.Scene) {
                packet.Scene->ForEachEntity([&](Entity* entity) {
                    ZoneScopedN("Entity Draw");
                    entity->OnDraw(m_RenderContext);
                });
            }
        }

        {
            ZoneScopedN("Debug Draw");
            Debug::Render(cmd, m_GlobalResource);
        }

        {
            ZoneScopedN("Editor Grid");
            m_RenderContext.CurrentShader = m_GridShader;
            cmd->UseShader(m_GridShader);
            cmd->BindResource(m_GlobalResource, m_GridShader, 0);
            // cmd->BindResource(m_SceneResource, m_GridShader, 1);
            cmd->Draw(6, 1);
        }
    }
    cmd->EndRenderPass(m_SceneRenderPass);
}
