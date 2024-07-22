#include "e5pch.h"
#include "MeshRenderer.h"

#include "OS/FileSystem.h"
#include "RHI/Device.h"
#include "RHI/FrameAllocator.h"
#include "RHI/Renderer.h"
#include "Scene/Entity.h"

void Fussion::MeshRenderer::OnCreate()
{

    m_Data.Model = glm::mat4(1.0f);
    m_FrameAllocator.Init(1);
}

void Fussion::MeshRenderer::OnUpdate(f32 delta) {}

void Fussion::MeshRenderer::OnDraw(RHI::RenderContext& ctx)
{
    if (!Mesh.IsValid())
        return;
    auto mesh = Mesh.Get();

    if (ctx.RenderFlags.Test(RHI::RenderState::Mesh)) {
        auto material = Material.Get();
        if (material == nullptr) {
            material = RHI::Renderer::GetDefaultMaterial().Get();
        }

        m_Data.Model = m_Owner->Transform.GetMatrix();
        m_FrameAllocator.Reset();

        std::vector resource_usages = {
            RHI::ResourceUsage{
                .Label = "Material",
                .Type = RHI::ResourceType::UniformBuffer,
                .Count = 1,
                .Stages = RHI::ShaderType::Vertex | RHI::ShaderType::Fragment,
            },
        };
        auto object_layout = RHI::Device::Instance()->CreateResourceLayout(resource_usages);
        auto resource = m_FrameAllocator.Alloc(object_layout);

        material->MaterialUniformBuffer.Data.ObjectColor = material->ObjectColor;
        material->MaterialUniformBuffer.Flush();

        ctx.Cmd->BindUniformBuffer(material->MaterialUniformBuffer.GetBuffer(), resource, 0);
        ctx.Cmd->BindResource(resource, ctx.CurrentShader, 1);
        ctx.Cmd->PushConstants(ctx.CurrentShader, &m_Data);
    } else if (ctx.RenderFlags.Test(RHI::RenderState::Depth)) {
        m_DepthPushData.Model = m_Owner->Transform.GetMatrix();
        m_DepthPushData.LightSpace = ctx.CurrentLightSpace;
        ctx.Cmd->PushConstants(ctx.CurrentShader, &m_DepthPushData);
    } else {
        return;
    }

    mesh->Draw(ctx);
}
