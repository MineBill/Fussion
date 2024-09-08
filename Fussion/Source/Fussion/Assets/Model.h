#pragma once
#include <Fussion/Assets/Asset.h>
#include <Fussion/Math/Vector2.h>
#include <Fussion/Math/Vector3.h>
#include <Fussion/Math/Vector4.h>
#include <Fussion/GPU/GPU.h>

namespace Fussion {
    struct Vertex {
        Vector3 position{};
        Vector3 normal{};
        Vector4 tangent{ 1, 1, 1 };
        Vector2 texture_coords{};
        Vector3 color{ 1, 1, 1 };
    };

    struct Mesh {
        std::vector<Vertex> vertices{};
        Vector3 offset{};

        GPU::Buffer vertex_buffer{};
        GPU::Buffer index_buffer{};
        // GPU::Buffer ShadowIndexBuffer{};
        GPU::Buffer instance_buffer{};
        u32 index_count{};

        s32 material_index{};

        Mesh(std::vector<Vertex> const& p_vertices, std::vector<u32> const& indices, std::vector<u32> const& shadow_indices, s32 material_index, Vector3 offset);
    };

    class Model final : public Asset {
    public:
        std::vector<Mesh> meshes{};
        u32 unique_materials{};

        static Ref<Model> create(std::vector<Mesh>& meshes);

        virtual AssetType type() const override { return static_type(); }

        static AssetType static_type() { return AssetType::Model; }
    };
}
