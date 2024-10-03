#pragma once
#include <Fussion/Assets/Asset.h>
#include <Fussion/GPU/GPU.h>
#include <Fussion/Math/BoundingBox.h>
#include <Fussion/Math/Vector2.h>
#include <Fussion/Math/Vector3.h>
#include <Fussion/Math/Vector4.h>

namespace Fussion {
    struct Vertex {
        Vector3 Position {};
        Vector3 Normal {};
        Vector4 Tangent { 1, 1, 1 };
        Vector2 TextureCoords {};
        Vector3 Color { 1, 1, 1 };
    };

    struct Mesh {
        std::vector<Vertex> Vertices {};
        Vector3 Offset {};
        BoundingBox Box {};

        GPU::Buffer VertexBuffer {};
        GPU::Buffer IndexBuffer {};
        // GPU::Buffer ShadowIndexBuffer{};
        GPU::Buffer InstanceBuffer {};
        u32 IndexCount {};

        s32 MaterialIndex {};

        Mesh(std::vector<Vertex> const& vertices, std::vector<u32> const& indices, std::vector<u32> const& shadow_indices, s32 material_index, Vector3 offset);
    };

    class Model final : public Asset {
    public:
        std::vector<Mesh> meshes {};
        u32 unique_materials {};

        static Ref<Model> Create(std::vector<Mesh>& meshes);

        virtual AssetType Type() const override { return StaticType(); }

        static AssetType StaticType() { return AssetType::Model; }
    };
}
