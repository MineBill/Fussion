#include "e5pch.h"
#include "MeshRenderer.h"

#include "OS/FileSystem.h"
#include "Renderer/Device.h"
#include "Renderer/FrameAllocator.h"
#include "Scene/Entity.h"

void Fussion::MeshRenderer::OnCreate()
{
    std::vector resource_usages = {
        ResourceUsage{
            .Label = "Object Resource",
            .Type = ResourceType::UniformBuffer,
            .Count = 1,
            .Stages = ShaderType::Vertex | ShaderType::Fragment,
        },
    };
    m_ObjectLayout = Device::Instance()->CreateResourceLayout(resource_usages);

    // m_FrameAllocator.Init(1);
    m_Data.Model = glm::mat4(1.0f);
}

void Fussion::MeshRenderer::OnUpdate(f32 delta)
{
}

void Fussion::MeshRenderer::OnDraw(RenderContext& ctx)
{
    if (!Mesh.IsValid())
        return;
    auto mesh = Mesh.Get();

    m_Data.Model = m_Owner->Transform.GetMatrix();
    // defer (m_FrameAllocator.Reset());
    //
    // auto resource = m_FrameAllocator.Alloc(m_ObjectLayout);
    // ctx.Cmd->BindResource(resource, ctx.CurrentShader, 1);
    ctx.Cmd->PushConstants(ctx.CurrentShader, &m_Data);
    mesh->Draw(ctx);
}
