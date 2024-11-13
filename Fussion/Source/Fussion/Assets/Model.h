#pragma once
#include <Fussion/Assets/Asset.h>
#include <Fussion/GPU/GPU.h>
#include <Fussion/Math/BoundingBox.h>
#include <Fussion/Math/Vector2.h>
#include <Fussion/Math/Vector3.h>
#include <Fussion/Math/Vector4.h>

namespace Fussion {
    struct Vertex {
        Vector3 position {};
        Vector3 normal {};
        Vector4 tangent { 1, 1, 1 };
        Vector2 texture_coords {};
        Vector3 color { 1, 1, 1 };
    };

    struct Mesh {
        std::vector<Vertex> vertices {};
        std::vector<u32> indices {};
        Vector3 offset {};
        BoundingBox box {};

        GPU::Buffer vertex_buffer {};
        GPU::Buffer index_buffer {};
        // GPU::Buffer ShadowIndexBuffer{};
        // GPU::Buffer InstanceBuffer {};
        u32 index_count {};

        s32 material_index {};

        Mesh(std::vector<Vertex> const& vertices, std::vector<u32> const& indices, std::vector<u32> const& shadow_indices, s32 material_index, Vector3 offset);

        static Mesh from_stream(std::istream& stream);

        void serialize(std::ostream& stream) const;
    };

    class Model final : public BinaryAsset {
    public:
        std::vector<Mesh> meshes {};
        u32 unique_material_count {};

        static Ref<Model> create(std::vector<Mesh>& meshes);

        virtual void serialize(std::ostream& stream) const override;
        virtual void deserialize(std::istream& stream) override;

        virtual AssetType type() const override { return static_type(); }
        static AssetType static_type() { return AssetType::Model; }
    };
}
