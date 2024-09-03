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

            RenderObject obj;
            obj.Material = material;
            obj.Position = m_Owner->Transform.Position;
            obj.WorldMatrix = translate(m_Owner->Transform.GetMatrix(), CAST(glm::vec3, mesh.Offset));
            obj.VertexBuffer = mesh.VertexBuffer;
            obj.IndexBuffer = mesh.IndexBuffer;
            obj.IndexCount = mesh.IndexCount;
            obj.InstanceBuffer = mesh.InstanceBuffer;
            obj.Pass = DrawPass::All;

            ctx.AddRenderObject(obj);
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
