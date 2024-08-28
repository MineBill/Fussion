#include "FussionPCH.h"
#include "MeshRenderer.h"

#include "Debug/Debug.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"
#include "RHI/Device.h"
#include "RHI/FrameAllocator.h"
#include "Rendering/Renderer.h"

#include <tracy/Tracy.hpp>

namespace Fussion {
    void MeshRenderer::OnStart()
    {
        ZoneScoped;

        m_Data.Model = glm::mat4(1.0f);
        m_FrameAllocator.Init(1);

        m_MaterialResourceUsages = {
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
            },
        };
    }

    void MeshRenderer::OnUpdate([[maybe_unused]] f32 delta) {}

    void MeshRenderer::OnDraw(RenderContext& ctx)
    {
        ZoneScoped;
        if (!m_Owner->IsEnabled())
            return;
        auto model = Model.Get();
        if (model == nullptr)
            return;

        if (ctx.RenderFlags.Test(RenderState::Mesh)) {
            m_FrameAllocator.Reset();

            Materials.resize(model->Meshes.size());
            for (auto& mesh : model->Meshes) {
                PbrMaterial* material = nullptr;
                if (mesh.MaterialIndex != -1) {
                    material = Materials.at(mesh.MaterialIndex).Get();
                } else {
                    if (!Materials.empty()) {
                        material = Materials.at(0).Get();
                    }
                }
                if (material == nullptr) {
                    material = Renderer::GetDefaultMaterial().Get();
                }

                m_Data.Model = translate(m_Owner->Transform.GetMatrix(), CAST(glm::vec3, mesh.Offset));

                auto object_layout = RHI::Device::Instance()->CreateResourceLayout(m_MaterialResourceUsages);
                auto resource = m_FrameAllocator.Alloc(object_layout, "MeshRenderer Material");

                material->MaterialUniformBuffer.Data.ObjectColor = material->ObjectColor;
                material->MaterialUniformBuffer.Data.Metallic = material->Metallic;
                material->MaterialUniformBuffer.Data.Roughness = material->Roughness;
                material->MaterialUniformBuffer.Flush();

                ctx.Cmd->BindUniformBuffer(material->MaterialUniformBuffer.GetBuffer(), resource, 0);
                ctx.Cmd->BindResource(resource, ctx.CurrentShader, 2);
                auto albedo = material->AlbedoMap.Get();
                if (!albedo) {
                    albedo = Renderer::WhiteTexture().Get();
                }

                auto normal = material->NormalMap.Get();
                if (!normal) {
                    normal = Renderer::DefaultNormalMap().Get();
                }

                auto ao = material->AmbientOcclusionMap.Get();
                if (!ao) {
                    ao = Renderer::WhiteTexture().Get();
                }

                auto metallic_roughness = material->MetallicRoughnessMap.Get();
                if (!metallic_roughness) {
                    metallic_roughness = Renderer::WhiteTexture().Get();
                }

                auto emissive = material->EmissiveMap.Get();
                if (!emissive) {
                    emissive = Renderer::BlackTexture().Get();
                }

                ctx.Cmd->BindImage(albedo->GetImage(), resource, 1);
                ctx.Cmd->BindImage(normal->GetImage(), resource, 2);

                ctx.Cmd->BindImage(ao->GetImage(), resource, 3);
                ctx.Cmd->BindImage(metallic_roughness->GetImage(), resource, 4);
                ctx.Cmd->BindImage(emissive->GetImage(), resource, 5);

                ctx.Cmd->PushConstants(ctx.CurrentShader, &m_Data);

                ctx.Cmd->BindBuffer(mesh.VertexBuffer);
                ctx.Cmd->BindBuffer(mesh.IndexBuffer);
                ctx.Cmd->DrawIndexed(mesh.IndexCount, 1);
            }
            return;
        }

        for (auto const& mesh : model->Meshes) {
            if (ctx.RenderFlags.Test(RenderState::Depth)) {
                m_DepthPushData.Model = translate(m_Owner->Transform.GetMatrix(), CAST(glm::vec3, mesh.Offset));
                m_DepthPushData.LightSpace = ctx.CurrentLightSpace;
                ctx.Cmd->PushConstants(ctx.CurrentShader, &m_DepthPushData);
            } else if (ctx.RenderFlags.Test(RenderState::ObjectPicking)) {
                m_ObjectPickingPushData.Model = translate(m_Owner->Transform.GetMatrix(), CAST(glm::vec3, mesh.Offset));
                m_ObjectPickingPushData.LocalID = m_Owner->GetSceneLocalID();
                ctx.Cmd->PushConstants(ctx.CurrentShader, &m_ObjectPickingPushData);
            } else {
                return;
            }

            ctx.Cmd->BindBuffer(mesh.VertexBuffer);
            ctx.Cmd->BindBuffer(mesh.IndexBuffer);
            ctx.Cmd->DrawIndexed(mesh.IndexCount, 1);
        }
    }

    void MeshRenderer::OnDebugDraw(DebugDrawContext& ctx)
    {
        if (!m_Owner->IsEnabled())
            return;
        auto draw_normals = ctx.Flags.Test(DebugDrawFlag::DrawMeshNormals);
        auto draw_tangents = ctx.Flags.Test(DebugDrawFlag::DrawMeshTangents);
        if (!draw_normals && !draw_tangents) {
            return;
        }
        auto model = Model.Get();
        if (!model)
            return;

        auto mat = glm::mat3(glm::eulerAngleZXY(
            glm::radians(m_Owner->Transform.EulerAngles.Z),
            glm::radians(m_Owner->Transform.EulerAngles.X),
            glm::radians(m_Owner->Transform.EulerAngles.Y)) * glm::scale(Mat4(1.0), CAST(glm::vec3, m_Owner->Transform.Scale)));

        for (auto const& mesh : model->Meshes) {
            for (auto const& vertex : mesh.Vertices) {
                auto base = Vector3(mat * vertex.Position) + m_Owner->Transform.Position;

                if (draw_normals) {
                    Debug::DrawLine(base, base + mat * vertex.Normal * 0.1f, 0, Color::Green);
                }
                if (draw_tangents) {
                    // Debug::DrawLine(base, base + mat * vertex.Tangent * 0.1f, 0, Color::SkyBlue);
                }
            }
        }
    }

    Ref<Component> MeshRenderer::Clone()
    {
        auto mr = MakeRef<MeshRenderer>();
        mr->Model = Model;
        mr->Materials = Materials;
        return mr;
    }

    void MeshRenderer::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(Model);
        ctx.WriteCollection("Materials", Materials);
    }

    void MeshRenderer::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(Model);
        ctx.ReadCollection("Materials", Materials);
    }
}
