#pragma once
#include <Fussion/Assets/Asset.h>
#include <Fussion/RHI/Buffer.h>
#include <Fussion/Rendering/RenderTypes.h>
#include <Fussion/Math/Vector2.h>
#include <Fussion/Math/Vector3.h>
#include <Fussion/Math/Vector4.h>

namespace Fussion {
    struct Vertex {
        Vector3 Position{};
        Vector3 Normal{};
        Vector4 Tangent{ 1, 1, 1 };
        Vector2 TextureCoords{};
        Vector3 Color{ 1, 1, 1 };
    };

    struct Mesh {
        std::vector<Vertex> Vertices{};
        Vector3 Offset{};

        Ref<RHI::Buffer> VertexBuffer{};
        Ref<RHI::Buffer> IndexBuffer{};
        Ref<RHI::Buffer> InstanceBuffer{};
        u32 IndexCount{};

        s32 MaterialIndex{};

        Mesh(std::vector<Vertex> const& vertices, std::vector<u32> const& indices, s32 material_index, Vector3 offset);
    };

    class Model final : public Asset {
    public:
        std::vector<Mesh> Meshes{};
        u32 UniqueMaterials{};

        static Ref<Model> Create(std::vector<Mesh>& meshes);

        virtual AssetType GetType() const override { return GetStaticType(); }

        static AssetType GetStaticType() { return AssetType::Model; }
    };
}
