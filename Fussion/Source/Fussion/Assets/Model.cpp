#include "FussionPCH.h"
#include "Model.h"

#include "GPU/GPU.h"
#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    Mesh::Mesh(std::vector<Vertex> const& vertices, std::vector<u32> const& indices, std::vector<u32> const& shadow_indices, s32 material_index, Vector3 offset)
        : Vertices(vertices)
        , Indices(indices)
        , Offset(offset)
        , MaterialIndex(material_index)
    {
        (void)shadow_indices;

        auto& device = Renderer::Device();

        auto vertex_spec = GPU::BufferSpec {
            .Label = "Mesh Vertex Buffer"sv,
            .Usage = GPU::BufferUsage::Vertex | GPU::BufferUsage::CopyDst,
            .Size = CAST(u32, Vertices.size() * sizeof(Vertex)),
        };
        VertexBuffer = device.CreateBuffer(vertex_spec);
        device.WriteBuffer(VertexBuffer, 0, std::span { Vertices });

        auto index_spec = GPU::BufferSpec {
            .Label = "Index Vertex Buffer"sv,
            .Usage = GPU::BufferUsage::Index | GPU::BufferUsage::CopyDst,
            .Size = CAST(u32, Indices.size() * sizeof(u32)),
        };
        IndexBuffer = device.CreateBuffer(index_spec);
        device.WriteBuffer(IndexBuffer, 0, std::span { Indices });

        IndexCount = CAST(u32, Indices.size());

        // auto instance_spec = GPU::BufferSpec{
        //     .Label = "Instance Buffer"sv,
        //     .Usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        //     .Size = sizeof(Mat4) * 1'000,
        // };
        //
        // InstanceBuffer = device.CreateBuffer(instance_spec);
    }

    Ref<Model> Model::Create(std::vector<Mesh>& meshes)
    {
        LOG_DEBUGF("Creating model with {} meshes", meshes.size());
        auto model = MakeRef<Model>();
        model->Meshes = std::move(meshes);
        return model;
    }

    void Model::Serialize(Serializer& ctx) const
    {
        Asset::Serialize(ctx);
        ctx.BeginArray("Meshes", Meshes.size());
        for (auto const& mesh : Meshes) {
            ctx.BeginObject("", 2);
            ctx.Write("MaterialIndex", mesh.MaterialIndex);
            ctx.Write("Offset", mesh.Offset);
            ctx.WriteByteArray("Vertices", TRANSMUTE(u8 const*, mesh.Vertices.data()), mesh.Vertices.size() * sizeof(Vertex));
            ctx.WriteByteArray("Indices", TRANSMUTE(u8 const*, mesh.Indices.data()), mesh.Indices.size() * sizeof(u32));
            ctx.EndObject();
        }
        ctx.EndArray();
    }

    void Model::Deserialize(Deserializer& ctx)
    {
        Asset::Deserialize(ctx);
        size_t size;
        ctx.BeginArray("Meshes", size);
        // Meshes.resize(size);
        for (size_t i = 0; i < size; ++i) {
            size_t objSize;
            ctx.BeginObject("", objSize);

            s32 materialIndex;
            Vector3 offset;
            ctx.Read("MaterialIndex", materialIndex);
            ctx.Read("Offset", offset);

            size_t verticesSize;
            std::vector<Vertex> vertices {};
            ctx.Read("", verticesSize);
            vertices.resize(verticesSize / sizeof(Vertex));
            ctx.ReadByteArray("Vertices", TRANSMUTE(u8*, vertices.data()), verticesSize);

            size_t indicesSize;
            std::vector<u32> indices {};
            ctx.Read("", indicesSize);
            indices.resize(indicesSize / sizeof(u32));
            ctx.ReadByteArray("Indices", TRANSMUTE(u8*, indices.data()), indicesSize);

            ctx.EndObject();

            Meshes.emplace_back(vertices, indices, std::vector<u32> {}, materialIndex, offset);
        }
        ctx.EndArray();
    }
}
