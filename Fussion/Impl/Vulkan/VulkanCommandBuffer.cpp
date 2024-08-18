#include "FussionPCH.h"
#include "VulkanCommandBuffer.h"

#include "Common.h"
#include "VulkanFrameBuffer.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include <Fussion/Core/SmallVector.h>

namespace Fussion::RHI {
    VulkanCommandBuffer::VulkanCommandBuffer(Ref<VulkanCommandPool> pool, CommandBufferSpecification spec)
    {
        Specification = spec;

        auto device = Device::Instance()->As<VulkanDevice>();
        auto ci = VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool->Handle,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VK_CHECK(vkAllocateCommandBuffers(device->Handle, &ci, &Handle))
    }

    void VulkanCommandBuffer::Begin(CommandBufferType type)
    {
        auto begin_info = VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };

        if (type == CommandBufferType::SingleTime) {
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }

        VK_CHECK(vkBeginCommandBuffer(Handle, &begin_info))
    }

    void VulkanCommandBuffer::End(CommandBufferType type)
    {
        (void)type;
        VK_CHECK(vkEndCommandBuffer(Handle))
    }

    void VulkanCommandBuffer::Reset()
    {
        vkResetCommandBuffer(Handle, {});
    }

    void VulkanCommandBuffer::BeginRenderPass(Ref<RenderPass> const render_pass, Ref<FrameBuffer> frame_buffer)
    {
        RenderPassStack.push(render_pass.get());

        SmallVector<VkClearValue, 5> clear_values;
        for (auto const& attachment : render_pass->GetSpec().Attachments) {
            if (Image::IsDepthFormat(attachment.Format)) {
                (void)clear_values.PushBack(VkClearValue{
                    .depthStencil = VkClearDepthStencilValue{
                        .depth = attachment.ClearDepth,
                        .stencil = attachment.ClearStencil,
                    },
                });
            } else {
                (void)clear_values.PushBack(VkClearValue{
                    .color = VkClearColorValue{
                        .float32 = { attachment.ClearColor[0], attachment.ClearColor[1], attachment.ClearColor[2], attachment.ClearColor[3] },
                    },
                });
            }
        }
        auto const& fb_spec = frame_buffer->GetSpec();
        auto info = VkRenderPassBeginInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = render_pass->GetRenderHandle<VkRenderPass>(),
            .framebuffer = frame_buffer->GetRenderHandle<VkFramebuffer>(),
            .renderArea = VkRect2D{
                .offset = VkOffset2D{
                    .x = 0, .y = 0,
                },
                .extent = VkExtent2D{
                    .width = CAST(u32, fb_spec.Width),
                    .height = CAST(u32, fb_spec.Height),
                },
            },
            .clearValueCount = CAST(u32, clear_values.Size()),
            .pClearValues = clear_values.Data(),
        };

        vkCmdBeginRenderPass(Handle, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCommandBuffer::EndRenderPass(const Ref<RenderPass> render_pass)
    {
        if (!RenderPassStack.top()->Equals(render_pass)) {
            LOG_ERRORF("Ended RenderPass('{}') out of order!", render_pass->GetSpec().Label);
        }
        RenderPassStack.pop();
        vkCmdEndRenderPass(Handle);
    }

    void VulkanCommandBuffer::UseShader(Ref<RHI::Shader> const& shader)
    {
        auto s = shader->As<VulkanShader>();
        auto handle = CAST(VkPipeline, shader->GetPipeline()->GetRenderHandle<VkPipeline>());
        vkCmdBindPipeline(Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, handle);
    }

    void VulkanCommandBuffer::SetScissor(const Vector4 size)
    {
        auto scissor = VkRect2D{
            .offset = VkOffset2D{
                .x = CAST(s32, size.X),
                .y = CAST(s32, size.Y),
            },
            .extent = VkExtent2D{
                .width = CAST(u32, size.Z),
                .height = CAST(u32, size.W),
            },
        };

        vkCmdSetScissor(Handle, 0, 1, &scissor);
    }

    void VulkanCommandBuffer::SetViewport(const Vector2 size)
    {
        auto viewport = VkViewport{
            .x = 0,
            .y = size.Y > 0 ? 0 : -size.Y,
            .width = size.X,
            .height = size.Y,
            .minDepth = 0,
            .maxDepth = 1,
        };

        vkCmdSetViewport(Handle, 0, 1, &viewport);
    }

    void VulkanCommandBuffer::Draw(u32 vertex_count, u32 instance_count)
    {
        vkCmdDraw(Handle, vertex_count, instance_count, 0, 0);
    }

    void VulkanCommandBuffer::DrawIndexed(u32 index_count, u32 instance_count)
    {
        vkCmdDrawIndexed(Handle, index_count, instance_count, 0, 0, 0);
    }

    void VulkanCommandBuffer::BindBuffer(Ref<Buffer> const& buffer)
    {
        auto const& spec = buffer->GetSpec();
        if (spec.Usage.Test(BufferUsage::Index)) {
            vkCmdBindIndexBuffer(Handle, buffer->GetRenderHandle<VkBuffer>(), 0, VK_INDEX_TYPE_UINT16);
        } else if (spec.Usage.Test(BufferUsage::Vertex)) {
            const auto handle = buffer->GetRenderHandle<VkBuffer>();
            constexpr auto offsets = VkDeviceSize{};
            vkCmdBindVertexBuffers(Handle, 0, 1, &handle, &offsets);
        }
    }

    void VulkanCommandBuffer::BindResource(Ref<Resource> const& resource, Ref<RHI::Shader> const& shader, u32 location)
    {
        const auto layout = shader->As<VulkanShader>()->GetPipeline()->GetLayout()->GetRenderHandle<VkPipelineLayout>();
        const VkDescriptorSet sets[] = {
            resource->GetRenderHandle<VkDescriptorSet>(),
        };
        vkCmdBindDescriptorSets(Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, location, 1, sets, 0, nullptr);
    }

    void VulkanCommandBuffer::BindImage(Ref<Image> const& image, Ref<Resource> const& resource, u32 location)
    {
        auto vk_image = image->As<VulkanImage>();
        auto image_info = VkDescriptorImageInfo{
            .sampler = vk_image->Sampler->GetRenderHandle<VkSampler>(),
            .imageView = vk_image->View->GetRenderHandle<VkImageView>(),
            .imageLayout = ImageLayoutToVulkan(vk_image->Specification.Layout != ImageLayout::Undefined ? vk_image->Specification.Layout : vk_image->Specification.FinalLayout)
        };

        auto write = VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = resource->GetRenderHandle<VkDescriptorSet>(),
            .dstBinding = location,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            // .pBufferInfo = &buffer_info,
            .pImageInfo = &image_info,
        };

        vkUpdateDescriptorSets(Device::Instance()->As<VulkanDevice>()->Handle, 1, &write, 0, nullptr);
    }

    void VulkanCommandBuffer::BindUniformBuffer(Ref<Buffer> const& buffer, Ref<Resource> const& resource, u32 location)
    {
        auto buffer_info = VkDescriptorBufferInfo{
            .buffer = buffer->GetRenderHandle<VkBuffer>(),
            .offset = 0,
            .range = VK_WHOLE_SIZE,
        };

        auto write = VkWriteDescriptorSet{
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

    void VulkanCommandBuffer::PushConstants(Ref<RHI::Shader> const& shader, void* data, size_t size)
    {
        auto layout = shader->GetPipeline()->GetLayout()->GetRenderHandle<VkPipelineLayout>();
        vkCmdPushConstants(Handle, layout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, CAST(u32, size), data);
    }

    void VulkanCommandBuffer::CopyImageToBuffer(Ref<Image> const& image, Ref<Buffer> const& buffer, Vector2 region)
    {
        VkBufferImageCopy copy{
            .imageSubresource = VkImageSubresourceLayers{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .layerCount = 1,
            },
            .imageExtent = VkExtent3D{
                .width = CAST(u32, region.X),
                .height = CAST(u32, region.Y),
                .depth = 1,
            }
        };

        vkCmdCopyImageToBuffer(Handle, image->GetRenderHandle<VkImage>(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->GetRenderHandle<VkBuffer>(), 1, &copy);
    }

    void VulkanCommandBuffer::BlitImage(
        Ref<Image> const& source_image,
        Ref<Image>& output_image,
        Rect source_rect,
        Rect dest_rect,
        u32 source_mip_level,
        u32 dest_mip_level)
    {
        VkImageBlit blit{
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = source_mip_level,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcOffsets = {
                {
                    .x = CAST(s32, source_rect.Position.X),
                    .y = CAST(s32, source_rect.Position.Y),
                    .z = 0
                },
                {
                    .x = CAST(s32, source_rect.Position.X + source_rect.Size.X),
                    .y = CAST(s32, source_rect.Position.Y + source_rect.Size.Y),
                    .z = 1
                }
            },
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = dest_mip_level,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .dstOffsets = {
                {
                    .x = CAST(s32, dest_rect.Position.X),
                    .y = CAST(s32, dest_rect.Position.Y),
                    .z = 0
                },
                {
                    .x = CAST(s32, dest_rect.Position.X + dest_rect.Size.X),
                    .y = CAST(s32, dest_rect.Position.Y + dest_rect.Size.Y),
                    .z = 1
                }
            },
        };

        auto source_layout = source_image->As<VulkanImage>()->GetLayoutForMipLevel(source_mip_level);
        auto dest_layout = output_image->As<VulkanImage>()->GetLayoutForMipLevel(dest_mip_level);

        vkCmdBlitImage(
            Handle,
            source_image->GetRenderHandle<VkImage>(), source_layout,
            output_image->GetRenderHandle<VkImage>(), dest_layout,
            1, &blit,
            VK_FILTER_LINEAR);
    }
}
