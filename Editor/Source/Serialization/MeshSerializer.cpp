#include "MeshSerializer.h"
#include "Project/Project.h"

#include "Fussion/Assets/Mesh.h"

#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
#include "Fussion/OS/FileSystem.h"

#include <vulkan/vulkan_core.h>

using namespace Fussion;

void MeshSerializer::Save(EditorAssetMetadata metadata, Ref<Asset> const& asset)
{
    (void)metadata;
    (void)asset;
}

Ref<Asset> MeshSerializer::Load(EditorAssetMetadata metadata)
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

    auto&  pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
    auto& pos_view = model.bufferViews[pos_accessor.bufferView];
    auto&  pos_buffer = model.buffers[pos_view.buffer];
    LOG_DEBUGF("pos: {} {}", pos_view.byteOffset, pos_accessor.byteOffset);


    auto& norm_accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
    auto& norm_view = model.bufferViews[norm_accessor.bufferView];
    auto& norm_buffer = model.buffers[norm_view.buffer];
    LOG_DEBUGF("nor: {} {}", norm_view.byteOffset, norm_accessor.byteOffset);

    auto& uv_accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
    VERIFY(uv_accessor.type == TINYGLTF_TYPE_VEC2);
    VERIFY(uv_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
    auto& uv_view = model.bufferViews[uv_accessor.bufferView];
    auto& uv_buffer = model.buffers[uv_view.buffer];
    LOG_DEBUGF("uvs: {} {}", uv_view.byteOffset, uv_accessor.byteOffset);

    auto& idx_accessor = model.accessors[primitive.indices];
    auto& idx_view = model.bufferViews[idx_accessor.bufferView];
    auto& idx_buffer = model.buffers[idx_view.buffer];

    auto pos_data = TRANSMUTE(const float*, &pos_buffer.data[pos_view.byteOffset + pos_accessor.byteOffset]);
    auto norm_data = TRANSMUTE(const float*, &norm_buffer.data[norm_view.byteOffset + norm_accessor.byteOffset]);
    auto uv_data = TRANSMUTE(const float*, &uv_buffer.data[uv_view.byteOffset + uv_accessor.byteOffset]);

    auto idx_data = TRANSMUTE(const u32*, &idx_buffer.data[idx_view.byteOffset + idx_accessor.byteOffset]);

    bool has_tangent = primitive.attributes.contains("TANGENT");
    f32 const* tangent_data{ nullptr };
    if (has_tangent) {
        LOG_DEBUGF("Mesh {} has tangents.", mesh.name);
        auto& accessor = model.accessors[primitive.attributes.find("TANGENT")->second];
        VERIFY(accessor.type == TINYGLTF_TYPE_VEC4);
        auto& view = model.bufferViews[accessor.bufferView];
        auto& buffer = model.buffers[view.buffer];
        tangent_data = TRANSMUTE(const f32*, &buffer.data[view.byteOffset + accessor.byteOffset]);
        LOG_DEBUGF("Tangent count: {}", accessor.count);
        LOG_DEBUGF("{} {}", view.byteOffset, accessor.byteOffset);
    }

    std::vector<Vertex> vertices;
    for (size_t i = 0; i < pos_accessor.count; i++) {
        Vertex vertex{};

        std::copy_n(pos_data + i * 3, 3, &vertex.Position.Raw[0]);
        std::copy_n(norm_data + i * 3, 3, &vertex.Normal.Raw[0]);
        std::copy_n(uv_data + i * 2, 2, &vertex.TextureCoords.Raw[0]);
        if (has_tangent) {
            std::copy_n(tangent_data + i * 4, 4, &vertex.Tangent.Raw[0]);
        }
        vertices.push_back(vertex);
    }

    std::vector<u32> indices;
    indices.assign(idx_data, idx_data + idx_accessor.count);

    if (!primitive.attributes.contains("TANGENT")) {
        LOG_WARNF("Mesh primitive[0] does not contain tangents.");

        // struct UserData {
        //     std::vector<u32>* Indices;
        //     std::vector<Vertex>* Vertices;
        //     tinygltf::Primitive* Primitive;
        // } context {
        //     .Indices = &indices,
        //     .Vertices = &vertices,
        //     .Primitive = &primitive,
        // };
        //
        // SMikkTSpaceContext ctx;
        // ctx.m_pUserData = &context;
        // ctx.m_pInterface->m_getNormal = [](SMikkTSpaceContext const* pContext, float fvNormOut[], const int iFace, const int iVert) {
        //     auto data = CAST(UserData*, pContext->m_pUserData);
        // };
        //
        // ctx.m_pInterface->m_getPosition = [](SMikkTSpaceContext const* pContext, float fvPosOut[], const int iFace, const int iVert) {
        //     auto data = CAST(UserData*, pContext->m_pUserData);
        // };
        //
        // ctx.m_pInterface->m_getNumFaces = [](SMikkTSpaceContext const* pContext) -> int {
        //     auto data = CAST(UserData*, pContext->m_pUserData);
        //     return data->Indices->size() / 3;
        // };
        //
        // ctx.m_pInterface->m_getTexCoord = [](SMikkTSpaceContext const* pContext, float fvTexcOut[], const int iFace, const int iVert) {
        //     auto data = CAST(UserData*, pContext->m_pUserData);
        //
        // };
        //
        // ctx.m_pInterface->m_setTSpaceBasic = [](const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert) {
        //     auto data = CAST(UserData*, pContext->m_pUserData);
        //
        // };
    }

    auto the_mesh = Mesh::Create(vertices, indices);
    return the_mesh;
}
