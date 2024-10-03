#include "FussionPCH.h"
#include "MeshRenderer.h"

#include "Debug/Debug.h"
#include "Rendering/Renderer.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <tracy/Tracy.hpp>

namespace Fussion {
    void MeshRenderer::OnStart()
    {
    }

    void MeshRenderer::OnUpdate([[maybe_unused]] f32 delta)
    {
        Model* model = ModelAsset.Get();
        if (!model)
            return;
        auto matrix = m_Owner->WorldMatrix();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        decompose(matrix, scale, rotation, translation, skew, perspective);

        m_Owner->GetBoundingBox() = BoundingBox(translation);
        for (auto const& mesh : model->meshes) {
            for (auto const& corner : mesh.Box.GetCorners()) {
                m_Owner->GetBoundingBox().IncludePoint(Vector3(matrix * Vector4(corner, 1.0f)));
            }
        }
    }

    void MeshRenderer::OnDraw(RenderContext& ctx)
    {
        ZoneScoped;
        if (!m_Owner->IsEnabled())
            return;
        auto m = ModelAsset.Get();
        if (m == nullptr)
            return;

        auto matrix = m_Owner->WorldMatrix();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        decompose(matrix, scale, rotation, translation, skew, perspective);

        m_Owner->GetBoundingBox() = BoundingBox(translation);
        for (auto const& mesh : m->meshes) {
            for (auto const& corner : mesh.Box.GetCorners()) {
                m_Owner->GetBoundingBox().IncludePoint(Vector3(matrix * Vector4(corner, 1.0f)));
            }
        }

        Materials.resize(m->meshes.size());
        for (auto& mesh : m->meshes) {
            PbrMaterial* material = nullptr;
            if (mesh.MaterialIndex != -1) {
                material = Materials.at(mesh.MaterialIndex).Get();
            } else {
                if (!Materials.empty()) {
                    material = Materials.at(0).Get();
                }
            }
            if (material == nullptr) {
                material = Renderer::DefaultMaterial().Get();
            }

            RenderObject obj;
            obj.Material = material;
            obj.Position = m_Owner->Transform.Position;
            obj.WorldMatrix = translate(matrix, CAST(glm::vec3, mesh.Offset));
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
    }

    Ref<Component> MeshRenderer::Clone()
    {
        auto mr = MakeRef<MeshRenderer>();
        mr->ModelAsset = ModelAsset;
        mr->Materials = Materials;
        return mr;
    }

    void MeshRenderer::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(ModelAsset);
        ctx.WriteCollection("materials", Materials);
    }

    void MeshRenderer::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(ModelAsset);
        ctx.ReadCollection("materials", Materials);

        // Trigger to calculate the bounding box.
        OnModelChanged();
    }

    void MeshRenderer::OnModelChanged()
    {
        Model* model = ModelAsset.Get();
        if (!model)
            return;

        auto matrix = m_Owner->WorldMatrix();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        decompose(matrix, scale, rotation, translation, skew, perspective);

        m_Owner->GetBoundingBox() = BoundingBox(translation);
        for (auto const& mesh : model->meshes) {
            for (auto const& corner : mesh.Box.GetCorners()) {
                m_Owner->GetBoundingBox().IncludePoint(Vector3(matrix * Vector4(corner, 1.0f)));
            }
        }
    }
}
