#pragma once
#include "VulkanDevice.h"
#include "Fussion/RHI/CommandBuffer.h"

#include "volk.h"
#include <stack>

namespace Fussion::RHI {
class VulkanCommandBuffer final : public CommandBuffer {
public:
    VulkanCommandBuffer(VulkanDevice* device, CommandBufferSpecification spec);

    virtual void Begin(CommandBufferType type) override;
    virtual void End(CommandBufferType type) override;
    virtual void Reset() override;
    virtual void BeginRenderPass(Ref<RenderPass> render_pass, Ref<FrameBuffer> frame_buffer) override;
    virtual void EndRenderPass(Ref<RenderPass> render_pass) override;

    virtual void UseShader(Ref<RHI::Shader> const& shader) override;

    virtual void SetScissor(Vector4) override;
    virtual void SetViewport(Vector2) override;

    virtual void Draw(u32 vertex_count, u32 instance_count) override;
    virtual void DrawIndexed(u32 index_count, u32 instance_count) override;

    virtual void BindBuffer(Ref<Buffer> const& buffer) override;
    virtual void BindResource(Ref<Resource> const& resource, Ref<RHI::Shader> const& shader, u32 location) override;

    virtual void BindImage(Ref<Image> const& image, Ref<Resource> const& resource, u32 location) override;
    virtual void BindUniformBuffer(Ref<Buffer> const& buffer, Ref<Resource> const& resource, u32 location) override;

    virtual void PushConstants(Ref<RHI::Shader> const& shader, void* data, size_t size) override;

    virtual void CopyImageToBuffer(Ref<Image> const& image, Ref<Buffer> const& buffer, Vector2 region) override;

    virtual void BlitImage(Ref<Image> const& source_image, Ref<Image>& output_image, Rect source_rect, Rect dest_rect) override;

    virtual void* GetRawHandle() override { return Handle; }

    VkCommandBuffer Handle{};
    std::stack<RenderPass*> RenderPassStack{};
};
}
