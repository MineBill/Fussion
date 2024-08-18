#include "FussionPCH.h"
#include "Mesh.h"

#include "RHI/Device.h"

namespace Fussion {
    Ref<Mesh> Mesh::Create(std::vector<Vertex> const& vertices, std::vector<u32> const& indices)
    {
        auto mesh = MakeRef<Mesh>();
        mesh->Vertices = vertices;

        auto device = RHI::Device::Instance();

        auto vertex_spec = RHI::BufferSpecification{
            .Label = "Mesh Vertex Buffer",
            .Usage = RHI::BufferUsage::Vertex,
            .Size = CAST(s32, vertices.size() * sizeof(Vertex)),
            .Mapped = true,
        };
        mesh->m_VertexBuffer = device->CreateBuffer(vertex_spec);
        mesh->m_VertexBuffer->SetData(std::span{ vertices });

        auto index_spec = RHI::BufferSpecification{
            .Label = "Index Vertex Buffer",
            .Usage = RHI::BufferUsage::Index,
            .Size = CAST(s32, indices.size() * sizeof(u32)),
            .Mapped = true,
        };
        mesh->m_IndexBuffer = device->CreateBuffer(index_spec);
        mesh->m_IndexBuffer->SetData(std::span{ indices });
        mesh->m_IndexCount = CAST(u32, indices.size());

        return mesh;
    }

    void Mesh::Draw(RHI::RenderContext& ctx)
    {
        auto& cmd = ctx.Cmd;

        cmd->BindBuffer(m_VertexBuffer);
        cmd->BindBuffer(m_IndexBuffer);
        cmd->DrawIndexed(m_IndexCount, 1);
    }
}
