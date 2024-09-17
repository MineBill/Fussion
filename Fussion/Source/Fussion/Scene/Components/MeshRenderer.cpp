#include "FussionPCH.h"
#include "MeshRenderer.h"

#include "Debug/Debug.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"
#include "Rendering/Renderer.h"

#include <tracy/Tracy.hpp>

namespace Fussion {
    void MeshRenderer::on_start() {}

    void MeshRenderer::on_update([[maybe_unused]] f32 delta) {}

    void MeshRenderer::on_draw(RenderContext& ctx)
    {
        ZoneScoped;
        if (!m_owner->is_enabled())
            return;
        auto m = model.get();
        if (m == nullptr)
            return;

        materials.resize(m->meshes.size());
        auto const world_matrix = m_owner->world_matrix();
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
            obj.world_matrix = translate(world_matrix, CAST(glm::vec3, mesh.offset));
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
        if (!m_owner->is_enabled())
            return;
        auto draw_normals = ctx.flags.test(DebugDrawFlag::DrawMeshNormals);
        auto draw_tangents = ctx.flags.test(DebugDrawFlag::DrawMeshTangents);
        if (!draw_normals && !draw_tangents) {
            return;
        }
        auto m = model.get();
        if (!m)
            return;

        auto mat = glm::mat3(glm::eulerAngleZXY(
            glm::radians(m_owner->transform.euler_angles.z),
            glm::radians(m_owner->transform.euler_angles.x),
            glm::radians(m_owner->transform.euler_angles.y)) * glm::scale(Mat4(1.0), CAST(glm::vec3, m_owner->transform.scale)));

        for (auto const& mesh : m->meshes) {
            for (auto const& vertex : mesh.vertices) {
                auto base = Vector3(mat * vertex.position) + m_owner->transform.position;

                if (draw_normals) {
                    Debug::draw_line(base, base + mat * vertex.normal * 0.1f, 0, Color::Green);
                }
                if (draw_tangents) {
                    // Debug::DrawLine(base, base + mat * vertex.Tangent * 0.1f, 0, Color::SkyBlue);
                }
            }
        }
    }

    Ref<Component> MeshRenderer::clone()
    {
        auto mr = make_ref<MeshRenderer>();
        mr->model = model;
        mr->materials = materials;
        return mr;
    }

    void MeshRenderer::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(model);
        ctx.write_collection("materials", materials);
    }

    void MeshRenderer::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(model);
        ctx.read_collection("materials", materials);
    }
}
