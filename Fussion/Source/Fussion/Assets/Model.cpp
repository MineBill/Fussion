#include "FussionPCH.h"
#include "Model.h"

#include "RHI/Device.h"

namespace Fussion {
    Mesh::Mesh(std::vector<Vertex> const& vertices, std::vector<u32> const& indices, s32 material_index, Vector3 offset)
        : Offset(offset),
          MaterialIndex(material_index)
    {
        Vertices = vertices;

        auto& device = RHI::Device::Instance();

        auto vertex_spec = RHI::BufferSpecification{
            .Label = "Mesh Vertex Buffer",
            .Usage = RHI::BufferUsage::Vertex,
            .Size = CAST(s32, vertices.size() * sizeof(Vertex)),
            .Mapped = true,
        };
        VertexBuffer = device->CreateBuffer(vertex_spec);
        VertexBuffer->SetData(std::span{ vertices });

        auto index_spec = RHI::BufferSpecification{
            .Label = "Index Vertex Buffer",
            .Usage = RHI::BufferUsage::Index,
            .Size = CAST(s32, indices.size() * sizeof(u32)),
            .Mapped = true,
        };
        IndexBuffer = device->CreateBuffer(index_spec);
        IndexBuffer->SetData(std::span{ indices });
        IndexCount = CAST(u32, indices.size());
    }

    Ref<Model> Model::Create(std::vector<Mesh>& meshes)
    {
        LOG_DEBUGF("Creating model with {} meshes", meshes.size());
        auto model = MakeRef<Model>();
        model->Meshes = std::move(meshes);
        return model;
    }
}
