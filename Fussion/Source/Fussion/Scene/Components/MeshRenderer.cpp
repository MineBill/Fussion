#include "e5pch.h"
#include "MeshRenderer.h"

#include "Debug/Debug.h"
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
    if (!m_Owner->IsEnabled())
        return;
    auto mesh = Mesh.Get();
    if (mesh == nullptr)
        return;

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
            RHI::ResourceUsage{
                .Label = "Albedo Map",
                .Type = RHI::ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = RHI::ShaderType::Fragment,
            },
            RHI::ResourceUsage{
                .Label = "Normal Map",
                .Type = RHI::ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = RHI::ShaderType::Fragment,
            },
            RHI::ResourceUsage{
                .Label = "AmbientOcclusion Map",
                .Type = RHI::ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = RHI::ShaderType::Fragment,
            },
            RHI::ResourceUsage{
                .Label = "MetallicRoughness Map",
                .Type = RHI::ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = RHI::ShaderType::Fragment,
            },
            RHI::ResourceUsage{
                .Label = "Emissive Map",
                .Type = RHI::ResourceType::CombinedImageSampler,
                .Count = 1,
                .Stages = RHI::ShaderType::Fragment,
            }
        };
        auto object_layout = RHI::Device::Instance()->CreateResourceLayout(resource_usages);
        auto resource = m_FrameAllocator.Alloc(object_layout, "MeshRenderer Material");

        material->MaterialUniformBuffer.Data.ObjectColor = material->ObjectColor;
        material->MaterialUniformBuffer.Data.Metallic = material->Metallic;
        material->MaterialUniformBuffer.Data.Roughness = material->Roughness;
        material->MaterialUniformBuffer.Flush();

        ctx.Cmd->BindUniformBuffer(material->MaterialUniformBuffer.GetBuffer(), resource, 0);
        ctx.Cmd->BindResource(resource, ctx.CurrentShader, 2);
        auto albedo = material->AlbedoMap.Get();
        if (!albedo) {
            albedo = RHI::Renderer::WhiteTexture().Get();
        }

        auto normal = material->NormalMap.Get();
        if (!normal) {
            normal = RHI::Renderer::DefaultNormalMap().Get();
        }

        auto ao = material->AmbientOcclusionMap.Get();
        if (!ao) {
            ao = RHI::Renderer::WhiteTexture().Get();
        }

        auto metallic_roughness = material->MetallicRoughnessMap.Get();
        if (!metallic_roughness) {
            metallic_roughness = RHI::Renderer::WhiteTexture().Get();
        }

        auto emissive = material->EmissiveMap.Get();
        if (!emissive) {
            emissive = RHI::Renderer::BlackTexture().Get();
        }

        ctx.Cmd->BindImage(albedo->GetImage(), resource, 1);
        ctx.Cmd->BindImage(normal->GetImage(), resource, 2);

        ctx.Cmd->BindImage(ao->GetImage(), resource, 3);
        ctx.Cmd->BindImage(metallic_roughness->GetImage(), resource, 4);
        ctx.Cmd->BindImage(emissive->GetImage(), resource, 5);

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

void Fussion::MeshRenderer::OnDebugDraw(DebugDrawContext& ctx)
{
    if (!m_Owner->IsEnabled())
        return;
    auto draw_normals = ctx.Flags.Test(DebugDrawFlag::DrawMeshNormals);
    auto draw_tangents = ctx.Flags.Test(DebugDrawFlag::DrawMeshTangents);
    if (!draw_normals && !draw_tangents) {
        return;
    }
    auto mesh = Mesh.Get();
    if (!mesh)
        return;

    auto mat = glm::mat3(glm::eulerAngleZXY(
        glm::radians(m_Owner->Transform.EulerAngles.Z),
        glm::radians(m_Owner->Transform.EulerAngles.X),
        glm::radians(m_Owner->Transform.EulerAngles.Y)) * glm::scale(Mat4(1.0), CAST(glm::vec3, m_Owner->Transform.Scale)));
    for (auto const& vertex : mesh->Vertices) {

        auto base = Vector3(mat * vertex.Position) + m_Owner->Transform.Position;

        if (draw_normals) {
            Debug::DrawLine(base, base + mat * vertex.Normal * 0.1f, 0, Color::Green);
        }
        if (draw_tangents) {
            // Debug::DrawLine(base, base + mat * vertex.Tangent * 0.1f, 0, Color::SkyBlue);
        }
    }
}
