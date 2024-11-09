#include "FussionPCH.h"
#include "Model.h"

#include "GPU/GPU.h"
#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    Mesh::Mesh(std::vector<Vertex> const& vertices, std::vector<u32> const& indices, std::vector<u32> const& shadow_indices, s32 material_index, Vector3 offset)
        : vertices(vertices)
        , indices(indices)
        , offset(offset)
        , material_index(material_index)
    {
        (void)shadow_indices;

        auto& device = Renderer::device();

        auto vertex_spec = GPU::BufferSpec {
            .label = "Mesh Vertex Buffer"sv,
            .usage = GPU::BufferUsage::Vertex | GPU::BufferUsage::CopyDst,
            .size = CAST(u32, vertices.size() * sizeof(Vertex)),
        };
        vertex_buffer = device.create_buffer(vertex_spec);
        device.write_buffer(vertex_buffer, 0, vertices);

        auto index_spec = GPU::BufferSpec {
            .label = "Index Vertex Buffer"sv,
            .usage = GPU::BufferUsage::Index | GPU::BufferUsage::CopyDst,
            .size = CAST(u32, indices.size() * sizeof(u32)),
        };
        index_buffer = device.create_buffer(index_spec);
        device.write_buffer(index_buffer, 0, indices);

        index_count = CAST(u32, indices.size());

        // auto instance_spec = GPU::BufferSpec{
        //     .Label = "Instance Buffer"sv,
        //     .Usage = GPU::BufferUsage::Storage | GPU::BufferUsage::CopyDst,
        //     .Size = sizeof(Mat4) * 1'000,
        // };
        //
        // InstanceBuffer = device.CreateBuffer(instance_spec);
    }

    Ref<Model> Model::create(std::vector<Mesh>& meshes)
    {
        LOG_DEBUGF("Creating model with {} meshes", meshes.size());
        auto model = make_ref<Model>();
        model->meshes = std::move(meshes);
        return model;
    }

    void Model::serialize(Serializer& ctx) const
    {
        Asset::serialize(ctx);
        ctx.begin_array("Meshes", meshes.size());
        for (auto const& mesh : meshes) {
            ctx.begin_object("", 2);
            ctx.write("MaterialIndex", mesh.material_index);
            ctx.write("Offset", mesh.offset);
            ctx.write_byte_array("Vertices", TRANSMUTE(u8 const*, mesh.vertices.data()), mesh.vertices.size() * sizeof(Vertex));
            ctx.write_byte_array("Indices", TRANSMUTE(u8 const*, mesh.indices.data()), mesh.indices.size() * sizeof(u32));
            ctx.end_object();
        }
        ctx.end_array();
    }

    void Model::deserialize(Deserializer& ctx)
    {
        Asset::deserialize(ctx);
        size_t size;
        ctx.begin_array("Meshes", size);
        // Meshes.resize(size);
        for (size_t i = 0; i < size; ++i) {
            size_t objSize;
            ctx.begin_object("", objSize);

            s32 materialIndex;
            Vector3 offset;
            ctx.read("MaterialIndex", materialIndex);
            ctx.read("Offset", offset);

            size_t verticesSize;
            std::vector<Vertex> vertices {};
            ctx.read("", verticesSize);
            vertices.resize(verticesSize / sizeof(Vertex));
            ctx.read_byte_array("Vertices", TRANSMUTE(u8*, vertices.data()), verticesSize);

            size_t indicesSize;
            std::vector<u32> indices {};
            ctx.read("", indicesSize);
            indices.resize(indicesSize / sizeof(u32));
            ctx.read_byte_array("Indices", TRANSMUTE(u8*, indices.data()), indicesSize);

            ctx.end_object();

            meshes.emplace_back(vertices, indices, std::vector<u32> {}, materialIndex, offset);
        }
        ctx.end_array();
    }
}
