#pragma once
#include "VulkanDevice.h"
#include "Engin5/Renderer/CommandBuffer.h"

#include "volk.h"
#include <stack>

namespace Engin5
{
    class VulkanCommandBuffer: public CommandBuffer
    {
    public:
        VulkanCommandBuffer(VulkanDevice* device, CommandBufferSpecification spec);

        void Begin(CommandBufferType type) override;
        void End(CommandBufferType type) override;
        void Reset() override;
        void BeginRenderPass(Ref<RenderPass> render_pass, Ref<FrameBuffer> frame_buffer) override;
        void EndRenderPass(Ref<RenderPass> render_pass) override;

        void UseShader(Ref<Shader> const& shader) override;

        void SetScissor(Vector4) override;
        void SetViewport(Vector2) override;

        void Draw(u32 vertex_count, u32 instance_count) override;
        void DrawIndexed(u32 vertex_count, u32 instance_count) override;

        void BindBuffer(Ref<Buffer> const& buffer) override;
        void BindResource(Ref<Resource> const& resource, Ref<Shader> const& shader, u32 location) override;
        void BindUniformBuffer(Ref<Buffer> const& buffer, Ref<Resource> const& resource, u32 location) override;

        void* GetRawHandle() override { return Handle; }

        VkCommandBuffer Handle{};
        std::stack<RenderPass*> RenderPassStack{};
    };
}
