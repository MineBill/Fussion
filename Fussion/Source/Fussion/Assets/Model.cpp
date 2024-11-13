#include "FussionPCH.h"
#include "Model.h"

#include "GPU/GPU.h"
#include "Rendering/Renderer.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    Mesh::Mesh(std::vector<Vertex> const& _vertices, std::vector<u32> const& _indices, std::vector<u32> const& _shadow_indices, s32 _material_index, Vector3 _offset)
        : vertices(_vertices)
        , indices(_indices)
        , offset(_offset)
        , material_index(_material_index)
    {
        (void)_shadow_indices;

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

    struct MeshHeader {
        u32 version = 1;
        u32 vertices_count;
        u32 index_count;
        s32 material_index;
    };

    Mesh Mesh::from_stream(std::istream& stream)
    {
        std::vector<Vertex> vertices {};
        std::vector<u32> indices {};
        Vector3 offset;

        MeshHeader header;
        stream.read(reinterpret_cast<char*>(&header), sizeof(MeshHeader));
        VERIFY(header.version == 1);
        vertices.resize(header.vertices_count);
        indices.resize(header.index_count);

        stream.read(reinterpret_cast<char*>(&offset), sizeof(Vector3));
        stream.read(reinterpret_cast<char*>(vertices.data()), header.vertices_count * sizeof(Vertex));
        stream.read(reinterpret_cast<char*>(indices.data()), header.index_count * sizeof(u32));

        return Mesh(vertices, indices, {}, header.material_index, offset);
    }

    void Mesh::serialize(std::ostream& stream) const
    {
        MeshHeader header;
        header.version = 1;
        header.vertices_count = cast<u32>(vertices.size());
        header.index_count = cast<u32>(indices.size());
        header.material_index = material_index;
        stream.write(reinterpret_cast<char const*>(&header), sizeof(MeshHeader));

        stream.write(reinterpret_cast<char const*>(&offset), sizeof(Vector3));
        stream.write(reinterpret_cast<char const*>(vertices.data()), vertices.size() * sizeof(Vertex));
        stream.write(reinterpret_cast<char const*>(indices.data()), indices.size() * sizeof(u32));
    }

    Ref<Model> Model::create(std::vector<Mesh>& meshes)
    {
        LOG_DEBUGF("Creating model with {} meshes", meshes.size());
        auto model = make_ref<Model>();
        model->meshes = std::move(meshes);
        return model;
    }

    struct ModelHeader {
        u32 version;
        u32 mesh_count;
    };

    void Model::serialize(std::ostream& stream) const
    {
        ModelHeader header;
        header.version = 1;
        header.mesh_count = cast<u32>(meshes.size());
        stream.write(reinterpret_cast<char const*>(&header), sizeof(header));

        for (auto const& mesh : meshes) {
            mesh.serialize(stream);
        }
        // Asset::serialize(ctx);
        // ctx.begin_array("Meshes", meshes.size());
        // for (auto const& mesh : meshes) {
        //     ctx.begin_object("", 2);
        //     ctx.write("MaterialIndex", mesh.material_index);
        //     ctx.write("Offset", mesh.offset);
        //     ctx.write_byte_array("Vertices", TRANSMUTE(u8 const*, mesh.vertices.data()), mesh.vertices.size() * sizeof(Vertex));
        //     ctx.write_byte_array("Indices", TRANSMUTE(u8 const*, mesh.indices.data()), mesh.indices.size() * sizeof(u32));
        //     ctx.end_object();
        // }
        // ctx.end_array();
    }

    void Model::deserialize(std::istream& stream)
    {
        ModelHeader header;
        stream.read(reinterpret_cast<char*>(&header), sizeof(header));
        VERIFY(header.version == 1);

        meshes.reserve(header.mesh_count);

        for ([[maybe_unused]] auto i : Range(0_u32, header.mesh_count - 1)) {
            meshes.push_back(Mesh::from_stream(stream));
        }
        // Asset::deserialize(ctx);
        // size_t size;
        // ctx.begin_array("Meshes", size);
        // // Meshes.resize(size);
        // for (size_t i = 0; i < size; ++i) {
        //     size_t objSize;
        //     ctx.begin_object("", objSize);
        //
        //     s32 materialIndex;
        //     Vector3 offset;
        //     ctx.read("MaterialIndex", materialIndex);
        //     ctx.read("Offset", offset);
        //
        //     size_t verticesSize;
        //     std::vector<Vertex> vertices {};
        //     ctx.read("", verticesSize);
        //     vertices.resize(verticesSize / sizeof(Vertex));
        //     ctx.read_byte_array("Vertices", TRANSMUTE(u8*, vertices.data()), verticesSize);
        //
        //     size_t indicesSize;
        //     std::vector<u32> indices {};
        //     ctx.read("", indicesSize);
        //     indices.resize(indicesSize / sizeof(u32));
        //     ctx.read_byte_array("Indices", TRANSMUTE(u8*, indices.data()), indicesSize);
        //
        //     ctx.end_object();
        //
        //     meshes.emplace_back(vertices, indices, std::vector<u32> {}, materialIndex, offset);
        // }
        // ctx.end_array();
    }
}
