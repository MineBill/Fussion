#include "EditorPCH.h"
#include "MeshSerializer.h"
#include "Project/Project.h"

#include <Fussion/Assets/Model.h>

#include "tiny_gltf.h"
#include <vulkan/vulkan_core.h>
#include "mikktspace.h"

using namespace Fussion;

namespace Mikktspace {
    struct UserData {
        std::vector<Vertex>* Vertices{};
        std::vector<u32>* Indices{};
    };

    int GetNumFaces(SMikkTSpaceContext const* ctx)
    {
        auto data = CAST(UserData*, ctx->m_pUserData);
        return CAST(int, data->Indices->size() / CAST(std::size_t, 3));
    }

    int GetNumVerticesOfFace(SMikkTSpaceContext const* ctx, int face)
    {
        (void)ctx;
        (void)face;
        return 3;
    }

    void GetPosition(SMikkTSpaceContext const* ctx, float pos_out[], int face, int vert)
    {
        auto data = CAST(UserData*, ctx->m_pUserData);
        auto index = data->Indices->at(face * 3 + vert);
        auto& vertex = data->Vertices->at(index);
        for (auto i = 0; i < 3; i++)
            pos_out[i] = vertex.Position[i];
    }

    void GetNormal(SMikkTSpaceContext const* ctx, float normal_out[], int face, int vert)
    {
        auto data = CAST(UserData*, ctx->m_pUserData);
        auto index = data->Indices->at(face * 3 + vert);
        auto& vertex = data->Vertices->at(index);
        for (auto i = 0; i < 3; i++)
            normal_out[i] = vertex.Normal[i];
    }

    void GetTexCoord(SMikkTSpaceContext const* ctx, float uv_out[], int face, int vert)
    {
        auto data = CAST(UserData*, ctx->m_pUserData);
        auto index = data->Indices->at(face * 3 + vert);
        auto& vertex = data->Vertices->at(index);
        for (auto i = 0; i < 2; i++)
            uv_out[i] = vertex.TextureCoords[i];
    }

    void SetTSpaceBasic(SMikkTSpaceContext const* ctx, float const tangent[], float sign, int face, int vert)
    {
        auto data = CAST(UserData*, ctx->m_pUserData);
        auto index = data->Indices->at(face * 3 + vert);
        auto& vertex = data->Vertices->at(index);

        for (auto i = 0; i < 3; i++)
            vertex.Tangent[i] = tangent[i];
        vertex.Tangent[3] = sign;
    }
}

void MeshSerializer::Save(EditorAssetMetadata metadata, Ref<Asset> const& asset)
{
    (void)metadata;
    (void)asset;
}

Ref<Asset> MeshSerializer::Load(EditorAssetMetadata metadata)
{
    auto path = Project::GetAssetsFolder() / metadata.Path;

    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    if (!loader.LoadBinaryFromFile(&model, &err, &warn, path.string())) {
        LOG_ERRORF("Failed to load GLB file: {} {}", err, warn);
        return nullptr;
    }

    LOG_DEBUGF("Using first mesh from glTF file");
    if (model.meshes.empty()) {
        LOG_DEBUGF("No meshes found in glTF file, ignoring..");
        return nullptr;
    }

    SMikkTSpaceInterface table{
        .m_getNumFaces = Mikktspace::GetNumFaces,
        .m_getNumVerticesOfFace = Mikktspace::GetNumVerticesOfFace,
        .m_getPosition = Mikktspace::GetPosition,
        .m_getNormal = Mikktspace::GetNormal,
        .m_getTexCoord = Mikktspace::GetTexCoord,
        .m_setTSpaceBasic = Mikktspace::SetTSpaceBasic,
        .m_setTSpace = nullptr,
    };

    SMikkTSpaceContext mikk_ctx;
    mikk_ctx.m_pInterface = &table;

    std::vector<Mesh> meshes;
    for (auto const& node : model.nodes) {
        if (node.mesh == -1)
            continue;

        auto mesh = model.meshes[node.mesh];
        auto primitive = mesh.primitives[0];

        auto& pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
        auto& pos_view = model.bufferViews[pos_accessor.bufferView];
        auto& pos_buffer = model.buffers[pos_view.buffer];

        auto& norm_accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
        auto& norm_view = model.bufferViews[norm_accessor.bufferView];
        auto& norm_buffer = model.buffers[norm_view.buffer];

        auto& uv_accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
        VERIFY(uv_accessor.type == TINYGLTF_TYPE_VEC2);
        VERIFY(uv_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        auto& uv_view = model.bufferViews[uv_accessor.bufferView];
        auto& uv_buffer = model.buffers[uv_view.buffer];

        auto& idx_accessor = model.accessors[primitive.indices];
        auto& idx_view = model.bufferViews[idx_accessor.bufferView];
        auto& idx_buffer = model.buffers[idx_view.buffer];

        auto pos_data = TRANSMUTE(const float*, &pos_buffer.data[pos_view.byteOffset + pos_accessor.byteOffset]);
        auto norm_data = TRANSMUTE(const float*, &norm_buffer.data[norm_view.byteOffset + norm_accessor.byteOffset]);
        auto uv_data = TRANSMUTE(const float*, &uv_buffer.data[uv_view.byteOffset + uv_accessor.byteOffset]);

        bool has_tangent = primitive.attributes.contains("TANGENT");
        f32 const* tangent_data{ nullptr };
        if (has_tangent) {
            auto& accessor = model.accessors[primitive.attributes.find("TANGENT")->second];
            VERIFY(accessor.type == TINYGLTF_TYPE_VEC4);
            auto& view = model.bufferViews[accessor.bufferView];
            auto& buffer = model.buffers[view.buffer];
            tangent_data = TRANSMUTE(const f32*, &buffer.data[view.byteOffset + accessor.byteOffset]);
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
        if (idx_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            auto idx_data = std::span(TRANSMUTE(const u16*, &idx_buffer.data[idx_view.byteOffset + idx_accessor.byteOffset]), idx_accessor.count);
            std::ranges::transform(idx_data, std::back_inserter(indices), [](u16 index) {
                return CAST(u32, index);
            });
        } else if (idx_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            indices.resize(idx_accessor.count);

            auto idx_data = std::span(TRANSMUTE(const u32*, &idx_buffer.data[idx_view.byteOffset + idx_accessor.byteOffset]), idx_accessor.count);
            indices.assign(idx_data.begin(), idx_data.end());
        }

        Mikktspace::UserData user_data{
            .Vertices = &vertices,
            .Indices = &indices,
        };
        mikk_ctx.m_pUserData = &user_data;

        if (!genTangSpaceDefault(&mikk_ctx)) {
            LOG_ERRORF("Failed to generate tangents, skipping mesh");
            continue;
        }

        meshes.emplace_back(vertices, indices, primitive.material);
    }

    auto the_model = Model::Create(meshes);
    the_model->UniqueMaterials = model.materials.size();
    return the_model;
}
