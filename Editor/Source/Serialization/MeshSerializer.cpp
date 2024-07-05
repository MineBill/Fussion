#include "MeshSerializer.h"
#include "Project/Project.h"

#include "Fussion/Assets/Mesh.h"

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
#include "Fussion/OS/FileSystem.h"

#include <vulkan/vulkan_core.h>

using namespace Fussion;

void MeshSerializer::Save(AssetMetadata metadata, Ref<Asset> const& asset)
{
    (void)metadata;
    (void)asset;
}

Ref<Asset> MeshSerializer::Load(AssetMetadata metadata)
{
    auto path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;

    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    // auto data = FileSystem::ReadEntireFileBinary(path);
    // if (!loader.LoadBinaryFromMemory(&model, &err, &warn, data.data(), data.size())) {
    if (!loader.LoadBinaryFromFile(&model, &err, &warn, path.string())) {
        LOG_ERRORF("Failed to load GLB file: {} {}", err, warn);
        return nullptr;
    }

    LOG_DEBUGF("Using first mesh from glTF file");
    if (model.meshes.empty()) {
        LOG_DEBUGF("No meshes found in glTF file, ignoring..");
        return nullptr;
    }
    auto mesh = model.meshes[0];
    auto primitive = mesh.primitives[0];

    auto pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
    auto pos_view = model.bufferViews[pos_accessor.bufferView];
    auto pos_buffer = model.buffers[pos_view.buffer];

    auto norm_accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
    auto norm_view = model.bufferViews[norm_accessor.bufferView];
    auto norm_buffer = model.buffers[norm_view.buffer];

    auto uv_accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
    auto uv_view = model.bufferViews[uv_accessor.bufferView];
    auto uv_buffer = model.buffers[uv_view.buffer];

    auto idx_accessor = model.accessors[primitive.indices];
    auto idx_view = model.bufferViews[idx_accessor.bufferView];
    auto idx_buffer = model.buffers[idx_view.buffer];

    auto pos_data = TRANSMUTE(const float*, &pos_buffer.data[pos_view.byteOffset + pos_accessor.byteOffset]);
    auto norm_data = TRANSMUTE(const float*, &norm_buffer.data[norm_view.byteOffset + norm_accessor.byteOffset]);
    auto uv_data = TRANSMUTE(const float*, &uv_buffer.data[uv_view.byteOffset + uv_accessor.byteOffset]);

    auto idx_data = TRANSMUTE(const u32*, &idx_buffer.data[idx_view.byteOffset + idx_accessor.byteOffset]);

    std::vector<Vertex> vertices;
    for (size_t i = 0; i < pos_accessor.count; i++) {
        Vertex vertex;

        std::copy_n(pos_data + i * 3, 3, &vertex.Position[0]);
        std::copy_n(norm_data + i * 3, 3, &vertex.Normal[0]);
        std::copy_n(uv_data + i * 2, 2, &vertex.Normal[0]);
        vertices.push_back(vertex);
    }

    std::vector<u32> indices;
    indices.assign(idx_data, idx_data + idx_accessor.count);

    auto the_mesh = Mesh::Create(vertices, indices);
    return the_mesh;
}
