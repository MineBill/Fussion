#include "SceneRenderer.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "Engin5/OS/FileSystem.h"
#include "Engin5/Renderer/Device.h"
#include "Engin5/Renderer/Renderer.h"
#include "Engin5/Renderer/ShaderCompiler.h"

void SceneRenderer::Init()
{
    using namespace Engin5;

    const auto scene_rp_spec = RenderPassSpecification {
        .Label = "Scene RenderPass",
        .Attachments = {
            RenderPassAttachment {
                .Label = "Color Attachment",
                .LoadOp = RenderPassAttachmentLoadOp::Clear,
                .StoreOp = RenderPassAttachmentStoreOp::Store,
                .Format = ImageFormat::B8G8R8A8_UNORM,
                .FinalLayout = ImageLayout::ColorAttachmentOptimal,
                .ClearColor = {0.2f, 0.6f, 0.15f, 1.0f},
            },
            RenderPassAttachment {
                .Label = "Depth Attachment",
                .LoadOp = RenderPassAttachmentLoadOp::Clear,
                .Format = ImageFormat::D32_SFLOAT,
                .FinalLayout = ImageLayout::DepthStencilAttachmentOptimal,
                .ClearDepth = 1.f,
            }
        },
        .SubPasses = {
            RenderPassSubPass {
                .ColorAttachments = {
                    {
                        .Attachment = 0,
                        .Layout = ImageLayout::ColorAttachmentOptimal,
                    }
                },
                .DepthStencilAttachment = RenderPassAttachmentRef {
                    .Attachment = 1,
                    .Layout = ImageLayout::DepthStencilAttachmentOptimal,
                }
            },
        }
    };

    m_SceneRenderPass = Device::Instance()->CreateRenderPass(scene_rp_spec);

    const auto fb_spec = FrameBufferSpecification {
        .Width = 400,
        .Height = 400,
        .Attachments = {
                FrameBufferAttachmentInfo {
                        .Format = ImageFormat::B8G8R8A8_UNORM,
                        .Usage = ImageUsage::Sampled | ImageUsage::ColorAttachment,
                },
                FrameBufferAttachmentInfo {
                        .Format = ImageFormat::D32_SFLOAT,
                        .Usage = ImageUsage::Sampled | ImageUsage::DepthStencilAttachment,
                }
        }
    };
    m_FrameBuffer = Device::Instance()->CreateFrameBuffer(m_SceneRenderPass, fb_spec);

    const auto data = FileSystem::ReadEntireFile("Assets/triangle.shader");
    auto [stages, metadata] = ShaderCompiler::Compile(data);
    m_TriangleShader = Device::Instance()->CreateShader(Renderer::GetInstance()->GetMainRenderPass(), stages, metadata);

    m_GlobalData = UniformBuffer<GlobalData>::Create("Global Data");

    const auto pool_spec = ResourcePoolSpecification::Default(100);
    m_ResourcePool = Device::Instance()->CreateResourcePool(pool_spec);

    std::vector resource_usages = {
        ResourceUsage{
            .Label = "awd",
            .Type = ResourceType::UniformBuffer,
            .Count = 1,
            .Stages = ShaderType::Vertex | ShaderType::Fragment,
        },
    };
    auto layout = Device::Instance()->CreateResourceLayout(resource_usages);
    auto result = m_ResourcePool->Allocate(layout);
    if (result.IsError()) {
        LOG_ERRORF("Error while allocating resource: {}", magic_enum::enum_name(result.Error()));
        return;
    }

    m_GlobalResource = result.TakeValue();
}

void SceneRenderer::Resize(const Vector2 new_size)
{
    ZoneScoped;
    Engin5::Device::Instance()->WaitIdle();
    m_RenderArea = new_size;
    m_FrameBuffer->Resize(new_size);
}

void SceneRenderer::Render(Ref<Engin5::CommandBuffer> const& cmd, RenderPacket const& packet)
{
    ZoneScoped;
    using namespace Engin5;

    m_GlobalData.Data.Perspective = packet.Camera.Perspective;
    m_GlobalData.Data.View = packet.Camera.View;
    m_GlobalData.Flush();

    cmd->BeginRenderPass(m_SceneRenderPass, m_FrameBuffer);
    {
        cmd->SetViewport({m_RenderArea.x, -m_RenderArea.y});
        cmd->SetScissor(Vector4(0, 0, m_RenderArea));
        cmd->UseShader(m_TriangleShader);
        cmd->BindResource(m_GlobalResource, m_TriangleShader, 0);
        cmd->BindUniformBuffer(m_GlobalData.GetBuffer(), m_GlobalResource, 0);
        cmd->Draw(6, 1);
    }
    cmd->EndRenderPass(m_SceneRenderPass);
}