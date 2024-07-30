#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/RHI/Buffer.h"
#include "Fussion/RHI/RenderContext.h"

namespace Fussion {

struct Vertex {
    Vector3 Position{};
    Vector3 Normal{};
    Vector3 Tangent{1, 1, 1};
    Vector2 TextureCoords{};
    Vector3 Color{ 1, 1, 1 };
};

class Mesh final : public Asset {
public:
    static Ref<Mesh> Create(std::vector<Vertex> const& vertices, std::vector<u32> const& indices);
    static AssetType GetStaticType() { return AssetType::Mesh; }
    AssetType GetType() const override { return GetStaticType(); }

    void Draw(RHI::RenderContext& ctx);

private:
    Ref<RHI::Buffer> m_VertexBuffer;
    Ref<RHI::Buffer> m_IndexBuffer;
    u32 m_IndexCount{};
};
}
