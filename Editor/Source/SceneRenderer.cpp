#include "SceneRenderer.h"

#include "Assets/Importers/TextureImporter.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "Fussion/OS/FileSystem.h"
#include "Fussion/Renderer/Device.h"
#include "Fussion/Renderer/Renderer.h"
#include "Fussion/Renderer/ShaderCompiler.h"
#include "Project/Project.h"

void SceneRenderer::Init()
{
    using namespace Fussion;

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

    {
        const auto data = FileSystem::ReadEntireFile("Assets/simple_3d.shader");
        auto [stages, metadata] = ShaderCompiler::Compile(*data);
        m_PbrShader = Device::Instance()->CreateShader(m_SceneRenderPass, stages, metadata);
    }

    {
        const auto data = FileSystem::ReadEntireFile("Assets/grid.shader");
        auto [stages, metadata] = ShaderCompiler::Compile(*data);
        metadata.UseBlending = true;
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

    m_TestTexture = TextureImporter::LoadTextureFromFile(std::filesystem::current_path() / "Assets" / "coords.png");
}

void SceneRenderer::Resize(const Vector2 new_size)
{
    ZoneScoped;
    Fussion::Device::Instance()->WaitIdle();
    m_RenderArea = new_size;
    m_FrameBuffer->Resize(new_size);
}

void SceneRenderer::Render(Ref<Fussion::CommandBuffer> const& cmd, RenderPacket const& packet)
{
    ZoneScoped;
    using namespace Fussion;

    m_ViewData.Data.Perspective = packet.Camera.Perspective;
    m_ViewData.Data.View = packet.Camera.View;
    m_ViewData.Flush();

    m_RenderContext.Cmd = cmd;
    cmd->BeginRenderPass(m_SceneRenderPass, m_FrameBuffer);
    {
        m_RenderContext.CurrentPass = m_SceneRenderPass;

        ZoneScopedN("Scene Render Pass");
        cmd->SetViewport({ m_RenderArea.X, -m_RenderArea.Y });
        cmd->SetScissor(Vector4(0, 0, m_RenderArea.X, m_RenderArea.Y));

        {
            ZoneScopedN("PBR");
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
