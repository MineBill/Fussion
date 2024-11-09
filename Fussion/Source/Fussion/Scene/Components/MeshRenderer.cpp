#include "FussionPCH.h"
#include "MeshRenderer.h"

#include "Debug/Debug.h"
#include "Rendering/Renderer.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <tracy/Tracy.hpp>

namespace Fussion {
    void MeshRenderer::on_start()
    {
    }

    void MeshRenderer::on_update([[maybe_unused]] f32 delta)
    {
        Model* model = ModelAsset.get();
        if (!model)
            return;
        auto matrix = m_owner->world_matrix();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        decompose(matrix, scale, rotation, translation, skew, perspective);

        m_owner->bounding_box() = BoundingBox(translation);
        for (auto const& mesh : model->meshes) {
            for (auto const& corner : mesh.box.corners()) {
                m_owner->bounding_box().add_point(Vector3(matrix * Vector4(corner, 1.0f)));
            }
        }
    }

    void MeshRenderer::on_draw(RenderContext& ctx)
    {
        ZoneScoped;
        if (!m_owner->enabled())
            return;
        auto m = ModelAsset.get();
        if (m == nullptr)
            return;

        auto matrix = m_owner->world_matrix();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        decompose(matrix, scale, rotation, translation, skew, perspective);

        m_owner->bounding_box() = BoundingBox(translation);
        for (auto const& mesh : m->meshes) {
            for (auto const& corner : mesh.box.corners()) {
                m_owner->bounding_box().add_point(Vector3(matrix * Vector4(corner, 1.0f)));
            }
        }

        Materials.resize(m->meshes.size());
        for (auto& mesh : m->meshes) {
            PbrMaterial* material = nullptr;
            if (mesh.material_index != -1) {
                material = Materials.at(mesh.material_index).get();
            } else {
                if (!Materials.empty()) {
                    material = Materials.at(0).get();
                }
            }
            if (material == nullptr) {
                material = Renderer::default_material().get();
            }

            RenderObject obj;
            obj.material = material;
            obj.position = m_owner->transform.Position;
            obj.world_matrix = translate(matrix, CAST(glm::vec3, mesh.offset));
            obj.vertex_buffer = mesh.vertex_buffer;
            obj.index_buffer = mesh.index_buffer;
            obj.index_count = mesh.index_count;
            // obj.InstanceBuffer = mesh.InstanceBuffer;
            obj.pass_flags = DrawPass::All;

            ctx.add_render_object(obj);
        }
    }

    void MeshRenderer::on_debug_draw(DebugDrawContext& ctx)
    {
        (void)ctx;
    }

    Ref<Component> MeshRenderer::clone()
    {
        auto mr = make_ref<MeshRenderer>();
        mr->ModelAsset = ModelAsset;
        mr->Materials = Materials;
        return mr;
    }

    void MeshRenderer::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(ModelAsset);
        ctx.write_collection("materials", Materials);
    }

    void MeshRenderer::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(ModelAsset);
        ctx.read_collection("materials", Materials);

        // Trigger to calculate the bounding box.
        on_model_changed();
    }

    void MeshRenderer::on_model_changed()
    {
        Model* model = ModelAsset.get();
        if (!model)
            return;

        auto matrix = m_owner->world_matrix();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        decompose(matrix, scale, rotation, translation, skew, perspective);

        m_owner->bounding_box() = BoundingBox(translation);
        for (auto const& mesh : model->meshes) {
            for (auto const& corner : mesh.box.corners()) {
                m_owner->bounding_box().add_point(Vector3(matrix * Vector4(corner, 1.0f)));
            }
        }
    }
}
