#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/Renderer/Buffer.h"
#include "Fussion/Renderer/RenderContext.h"

namespace Fussion {

struct Vertex {
    Vector3 Position;
    Vector3 Normal;
    Vector2 TextureCoords;
};

class Mesh final : public Asset {
public:
    static Ref<Mesh> Create(std::vector<Vertex> const& vertices, std::vector<u32> const& indices);
    static AssetType GetStaticType() { return AssetType::Mesh; }

    void Draw(RenderContext& ctx);
private:
    Ref<Buffer> m_VertexBuffer;
    Ref<Buffer> m_IndexBuffer;
    u32 m_IndexCount{};
};
}
