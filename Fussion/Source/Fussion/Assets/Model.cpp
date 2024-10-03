#include "Model.h"
#include "FussionPCH.h"

#include "../../../../build/.packages/m/meshoptimizer/latest/45f33ceed5a5435d86c88631b393e9c3/include/meshoptimizer.h"
#include "GPU/GPU.h"
#include "Rendering/Renderer.h"

namespace Fussion {
    Mesh::Mesh(std::vector<Vertex> const& vertices, std::vector<u32> const& indices, std::vector<u32> const& shadow_indices, s32 material_index, Vector3 offset)
        : Offset(offset),
          MaterialIndex(material_index)
    {
        (void)shadow_indices;
        Vertices = vertices;

        auto& device = Renderer::Device();

        auto vertex_spec = GPU::BufferSpec{
            .Label = "Mesh Vertex Buffer"sv,
            .Usage = GPU::BufferUsage::Vertex | GPU::BufferUsage::CopyDst,
            .Size = CAST(u32, Vertices.size() * sizeof(Vertex)),
        };
        VertexBuffer = device.CreateBuffer(vertex_spec);
        device.WriteBuffer(VertexBuffer, 0, std::span{ Vertices });

        auto index_spec = GPU::BufferSpec{
            .Label = "Index Vertex Buffer"sv,
            .Usage = GPU::BufferUsage::Index | GPU::BufferUsage::CopyDst,
            .Size = CAST(u32, indices.size() * sizeof(u32)),
        };
        IndexBuffer = device.CreateBuffer(index_spec);
        device.WriteBuffer(IndexBuffer, 0, std::span{ indices });

        IndexCount = CAST(u32, indices.size());

        auto instance_spec = GPU::BufferSpec{
            .Label = "Instance Buffer"sv,
            .Usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
            .Size = sizeof(Mat4) * 1'000,
        };

        InstanceBuffer = device.CreateBuffer(instance_spec);
    }

    Ref<Model> Model::Create(std::vector<Mesh>& meshes)
    {
        LOG_DEBUGF("Creating model with {} meshes", meshes.size());
        auto model = MakeRef<Model>();
        model->meshes = std::move(meshes);
        return model;
    }
}
