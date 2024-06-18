#pragma once
#include "FrameBuffer.h"
#include "RenderPass.h"
#include "Shader.h"
#include "Buffer.h"
#include "Fussion/Core/Types.h"
#include "Resources/Resource.h"

namespace Fussion {
enum class CommandBufferType {
    None,
    SingleTime,
};

struct CommandBufferSpecification {
    std::string Label;
};

class CommandBuffer : public RenderHandle {
public:
    virtual void Begin(CommandBufferType type = CommandBufferType::None) = 0;
    virtual void End(CommandBufferType type = CommandBufferType::None) = 0;

    virtual void BeginRenderPass(Ref<RenderPass> render_pass, Ref<FrameBuffer> frame_buffer) = 0;
    virtual void EndRenderPass(Ref<RenderPass> render_pass) = 0;

    virtual void UseShader(Ref<Shader> const& shader) = 0;

    virtual void Reset() = 0;
    virtual void SetViewport(Vector2 size) = 0;
    virtual void SetScissor(Vector4 size) = 0;

    virtual void Draw(u32 vertex_count, u32 instance_count) = 0;
    virtual void DrawIndexed(u32 vertex_count, u32 instance_count) = 0;

    virtual void BindBuffer(Ref<Buffer> const& buffer) = 0;
    virtual void BindResource(Ref<Resource> const& resource, Ref<Shader> const& shader, u32 location) = 0;

    virtual void BindImage(Ref<Image> const& image, Ref<Resource> const& resource, u32 location) = 0;
    virtual void BindUniformBuffer(Ref<Buffer> const& buffer, Ref<Resource> const& resource, u32 location) = 0;

    // virtual void BindBuffer() = 0;
    // virtual void BindResource() = 0;

    CommandBufferSpecification Specification;
};
}

namespace Fsn = Fussion;
