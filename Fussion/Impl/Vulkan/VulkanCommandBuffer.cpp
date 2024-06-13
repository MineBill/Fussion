#include "e5pch.h"
#include "VulkanCommandBuffer.h"

#include "Common.h"
#include "VulkanFrameBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "Resources/VulkanResourcePool.h"

Fussion::VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, CommandBufferSpecification spec)
{
    Specification = spec;

    auto ci = VkCommandBufferAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = device->CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VK_CHECK(vkAllocateCommandBuffers(device->Handle, &ci, &Handle))
}

void Fussion::VulkanCommandBuffer::Begin(CommandBufferType type)
{
    auto begin_info = VkCommandBufferBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    if (type == CommandBufferType::SingleTime) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(Handle, &begin_info))
}

void Fussion::VulkanCommandBuffer::End(const CommandBufferType type)
{
    (void)type;
    VK_CHECK(vkEndCommandBuffer(Handle))
}

void Fussion::VulkanCommandBuffer::Reset()
{
    vkResetCommandBuffer(Handle, {});
}

void Fussion::VulkanCommandBuffer::BeginRenderPass(const Ref<RenderPass> render_pass, Ref<FrameBuffer> frame_buffer)
{
    RenderPassStack.push(render_pass.get());

    std::vector<VkClearValue> clear_values;
    for (const auto& attachment : render_pass->GetSpec().Attachments) {
        if (Image::IsDepthFormat(attachment.Format)) {
            clear_values.push_back(VkClearValue {
                .depthStencil = VkClearDepthStencilValue {
                    .depth = attachment.ClearDepth,
                    .stencil = attachment.ClearStencil,
                },
            });
        } else {
            clear_values.push_back(VkClearValue {
                .color = VkClearColorValue {
                    .float32 = {attachment.ClearColor[0], attachment.ClearColor[1], attachment.ClearColor[2], attachment.ClearColor[3]},
                },
            });
        }
    }
    const auto fb_spec = frame_buffer->GetSpec();
    auto info = VkRenderPassBeginInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass->GetRenderHandle<VkRenderPass>(),
        .framebuffer = frame_buffer->GetRenderHandle<VkFramebuffer>(),
        .renderArea = VkRect2D {
            .offset = VkOffset2D {
                .x = 0, .y = 0,
            },
            .extent = VkExtent2D {
                .width = cast(u32, fb_spec.Width),
                .height = cast(u32, fb_spec.Height),
            },
        },
        .clearValueCount = cast(u32, clear_values.size()),
        .pClearValues = clear_values.data(),
    };

    vkCmdBeginRenderPass(Handle, &info, VK_SUBPASS_CONTENTS_INLINE);
}

void Fussion::VulkanCommandBuffer::EndRenderPass(const Ref<RenderPass> render_pass)
{
    if (!RenderPassStack.top()->Equals(render_pass)) {
        LOG_ERRORF("Ended RenderPass('{}') out of order!", render_pass->GetSpec().Label);
    }
    RenderPassStack.pop();
    vkCmdEndRenderPass(Handle);
}

void Fussion::VulkanCommandBuffer::UseShader(Ref<Shader> const& shader)
{
    auto s = shader->As<VulkanShader>();
    auto handle = cast(VkPipeline, shader->GetPipeline()->GetRenderHandle<VkPipeline>());
    vkCmdBindPipeline(Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, handle);
}

void Fussion::VulkanCommandBuffer::SetScissor(const Vector4 size)
{
    auto scissor = VkRect2D {
        .offset = VkOffset2D {
            .x = cast(s32, size.x),
            .y = cast(s32, size.y),
        },
        .extent = VkExtent2D {
            .width = cast(u32, size.z),
            .height = cast(u32, size.w),
        },
    };

    vkCmdSetScissor(Handle, 0, 1, &scissor);
}

void Fussion::VulkanCommandBuffer::SetViewport(const Vector2 size)
{
    auto viewport = VkViewport {
        .x = 0,
        .y = size.y > 0 ? 0 : -size.y,
        .width = size.x,
        .height = size.y,
        .minDepth = 0,
        .maxDepth = 1,
    };

    vkCmdSetViewport(Handle, 0, 1, &viewport);
}

void Fussion::VulkanCommandBuffer::Draw(u32 vertex_count, u32 instance_count)
{
    vkCmdDraw(Handle, vertex_count, instance_count, 0, 0);
}

void Fussion::VulkanCommandBuffer::DrawIndexed(u32 vertex_count, u32 instance_count)
{
    (void)vertex_count;
    (void)instance_count;
}

void Fussion::VulkanCommandBuffer::BindBuffer(Ref<Buffer> const& buffer)
{
    const auto spec = buffer->GetSpec();
    if (spec.Usage.Test(BufferUsage::Index)) {
        vkCmdBindIndexBuffer(Handle, buffer->GetRenderHandle<VkBuffer>(), 0, VK_INDEX_TYPE_UINT16);
    } else if (spec.Usage.Test(BufferUsage::Vertex)) {
        const auto handle = buffer->GetRenderHandle<VkBuffer>();
        constexpr auto offsets = VkDeviceSize{};
        vkCmdBindVertexBuffers(Handle, 0, 1, &handle, &offsets);
    }
}

void Fussion::VulkanCommandBuffer::BindResource(Ref<Resource> const& resource, Ref<Shader> const& shader, u32 location)
{
    const auto layout = shader->As<VulkanShader>()->GetPipeline()->GetLayout()->GetRenderHandle<VkPipelineLayout>();
    const VkDescriptorSet sets[] = {
        resource->GetRenderHandle<VkDescriptorSet>(),
    };
    vkCmdBindDescriptorSets(Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, location, 1, sets, 0, nullptr);
}

void Fussion::VulkanCommandBuffer::BindUniformBuffer(Ref<Buffer> const& buffer, Ref<Resource> const& resource, u32 location)
{
    auto buffer_info = VkDescriptorBufferInfo {
        .buffer = buffer->GetRenderHandle<VkBuffer>(),
        .offset = 0,
        .range = VK_WHOLE_SIZE,
    };

    auto write = VkWriteDescriptorSet {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = resource->GetRenderHandle<VkDescriptorSet>(),
        .dstBinding = location,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &buffer_info,
    };

    vkUpdateDescriptorSets(Device::Instance()->As<VulkanDevice>()->Handle, 1, &write, 0, nullptr);
}