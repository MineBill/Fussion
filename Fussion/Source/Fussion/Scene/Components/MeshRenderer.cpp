#include "e5pch.h"
#include "MeshRenderer.h"

#include "OS/FileSystem.h"
#include "RHI/Device.h"
#include "RHI/FrameAllocator.h"
#include "RHI/Renderer.h"
#include "Scene/Entity.h"
#include <tracy/Tracy.hpp>

void Fussion::MeshRenderer::OnStart()
{
    ZoneScoped;

    m_Data.Model = glm::mat4(1.0f);
    m_FrameAllocator.Init(1);
}

void Fussion::MeshRenderer::OnUpdate([[maybe_unused]] f32 delta) {}

void Fussion::MeshRenderer::OnDraw(RHI::RenderContext& ctx)
{
    ZoneScoped;
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

        std::array resource_usages = {
            RHI::ResourceUsage{
                .Label = "Material",
                .Type = RHI::ResourceType::UniformBuffer,
                .Count = 1,
                .Stages = RHI::ShaderType::Vertex | RHI::ShaderType::Fragment,
            },
        };
        auto object_layout = RHI::Device::Instance()->CreateResourceLayout(resource_usages);
        auto resource = m_FrameAllocator.Alloc(object_layout, "MeshRenderer Material");

        material->MaterialUniformBuffer.Data.ObjectColor = material->ObjectColor;
        material->MaterialUniformBuffer.Data.Metallic = material->Metallic;
        material->MaterialUniformBuffer.Data.Roughness = material->Roughness;
        material->MaterialUniformBuffer.Flush();

        ctx.Cmd->BindUniformBuffer(material->MaterialUniformBuffer.GetBuffer(), resource, 0);
        ctx.Cmd->BindResource(resource, ctx.CurrentShader, 2);
        ctx.Cmd->PushConstants(ctx.CurrentShader, &m_Data);
    } else if (ctx.RenderFlags.Test(RHI::RenderState::Depth)) {
        m_DepthPushData.Model = m_Owner->Transform.GetMatrix();
        m_DepthPushData.LightSpace = ctx.CurrentLightSpace;
        ctx.Cmd->PushConstants(ctx.CurrentShader, &m_DepthPushData);
    } else if (ctx.RenderFlags.Test(RHI::RenderState::ObjectPicking)) {
        m_ObjectPickingPushData.Model = m_Owner->Transform.GetMatrix();
        m_ObjectPickingPushData.LocalID = m_Owner->GetLocalID();
        ctx.Cmd->PushConstants(ctx.CurrentShader, &m_ObjectPickingPushData);
    } else {
        return;
    }

    mesh->Draw(ctx);
}
