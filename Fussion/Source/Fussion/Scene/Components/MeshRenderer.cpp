#include "MeshRenderer.h"

#include "Debug/Debug.h"
#include "FussionPCH.h"
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
        Model* model = model_asset.get();
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
            for (auto const& corner : mesh.bounding_box.get_corners()) {
                m_owner->bounding_box().include_point(Vector3(matrix * Vector4(corner, 1.0f)));
            }
        }
    }

    void MeshRenderer::on_draw(RenderContext& ctx)
    {
        ZoneScoped;
        if (!m_owner->is_enabled())
            return;
        auto m = model_asset.get();
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
            for (auto const& corner : mesh.bounding_box.get_corners()) {
                m_owner->bounding_box().include_point(Vector3(matrix * Vector4(corner, 1.0f)));
            }
        }

        materials.resize(m->meshes.size());
        for (auto& mesh : m->meshes) {
            PbrMaterial* material = nullptr;
            if (mesh.material_index != -1) {
                material = materials.at(mesh.material_index).get();
            } else {
                if (!materials.empty()) {
                    material = materials.at(0).get();
                }
            }
            if (material == nullptr) {
                material = Renderer::default_material().get();
            }

            RenderObject obj;
            obj.material = material;
            obj.position = m_owner->transform.position;
            obj.world_matrix = translate(matrix, CAST(glm::vec3, mesh.offset));
            obj.vertex_buffer = mesh.vertex_buffer;
            obj.index_buffer = mesh.index_buffer;
            obj.index_count = mesh.index_count;
            obj.instance_buffer = mesh.instance_buffer;
            obj.pass = DrawPass::All;

            ctx.add_render_object(obj);
        }
    }

    void MeshRenderer::on_debug_draw(DebugDrawContext& ctx)
    {
    }

    Ref<Component> MeshRenderer::clone()
    {
        auto mr = make_ref<MeshRenderer>();
        mr->model_asset = model_asset;
        mr->materials = materials;
        return mr;
    }

    void MeshRenderer::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(model_asset);
        ctx.write_collection("materials", materials);
    }

    void MeshRenderer::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(model_asset);
        ctx.read_collection("materials", materials);

        // Trigger to calculate the bounding box.
        model_changed();
    }

    void MeshRenderer::model_changed()
    {
        Model* model = model_asset.get();
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
            for (auto const& corner : mesh.bounding_box.get_corners()) {
                m_owner->bounding_box().include_point(Vector3(matrix * Vector4(corner, 1.0f)));
            }
        }
    }
}
