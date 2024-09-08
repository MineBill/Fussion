#include "FussionPCH.h"
#include "Model.h"
#include "GPU/GPU.h"
#include "Rendering/Renderer.h"

namespace Fussion {
    Mesh::Mesh(std::vector<Vertex> const& p_vertices, std::vector<u32> const& indices, std::vector<u32> const& shadow_indices, s32 material_index, Vector3 offset)
        : offset(offset),
          material_index(material_index)
    {
        (void)shadow_indices;
        vertices = p_vertices;

        auto& device = Renderer::device();

        auto vertex_spec = GPU::BufferSpec{
            .label = "Mesh Vertex Buffer"sv,
            .usage = GPU::BufferUsage::Vertex | GPU::BufferUsage::CopyDst,
            .size = CAST(u32, vertices.size() * sizeof(Vertex)),
        };
        vertex_buffer = device.create_buffer(vertex_spec);
        device.write_buffer(vertex_buffer, 0, std::span{ vertices });

        auto index_spec = GPU::BufferSpec{
            .label = "Index Vertex Buffer"sv,
            .usage = GPU::BufferUsage::Index | GPU::BufferUsage::CopyDst,
            .size = CAST(u32, indices.size() * sizeof(u32)),
        };
        index_buffer = device.create_buffer(index_spec);
        device.write_buffer(index_buffer, 0, std::span{ indices });

        index_count = CAST(u32, indices.size());

        auto instance_spec = GPU::BufferSpec{
            .label = "Instance Buffer"sv,
            .usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
            .size = sizeof(Mat4) * 1'000,
        };

        instance_buffer = device.create_buffer(instance_spec);
    }

    Ref<Model> Model::create(std::vector<Mesh>& meshes)
    {
        LOG_DEBUGF("Creating model with {} meshes", meshes.size());
        auto model = make_ref<Model>();
        model->meshes = std::move(meshes);
        return model;
    }
}
