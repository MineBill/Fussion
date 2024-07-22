#include "e5pch.h"
#include "VulkanRenderPass.h"

#include "Common.h"
#include "VulkanImage.h"

namespace Fussion::RHI {
VkAttachmentLoadOp LoadOpToVulkan(RenderPassAttachmentLoadOp op)
{
    switch (op) {
        using enum RenderPassAttachmentLoadOp;
    case DontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    case Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    }

    // @note Find some cross-platform to panic here but also include a return for compilers
    // that shit themselves if they don't see one (clang I think).
    return {};
}

VkAttachmentStoreOp StoreOpToVulkan(RenderPassAttachmentStoreOp op)
{
    switch (op) {
        using enum RenderPassAttachmentStoreOp;
    case DontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    }

    // @note Find some cross-platform to panic here but also include a return for compilers
    // that shit themselves if they don't see one (clang I think).
    return {};
}

VkSampleCountFlagBits SamplesToVulkan(s32 count)
{
    switch (count) {
    case 1:
        return VK_SAMPLE_COUNT_1_BIT;
    case 2:
        return VK_SAMPLE_COUNT_2_BIT;
    case 4:
        return VK_SAMPLE_COUNT_4_BIT;
    case 8:
        return VK_SAMPLE_COUNT_8_BIT;
    case 16:
        return VK_SAMPLE_COUNT_16_BIT;
    case 32:
        return VK_SAMPLE_COUNT_32_BIT;
    case 64:
        return VK_SAMPLE_COUNT_64_BIT;
    default:
        VERIFY(false, "Invalid sample count of {}", count)
    }
    return {};
}

VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, RenderPassSpecification spec)
    : m_Specification(spec)
{
    std::vector<VkAttachmentDescription> attachments;
    attachments.reserve(spec.Attachments.size());
    for (const auto& attachment : spec.Attachments) {
        auto info = VkAttachmentDescription{
            .format = ImageFormatToVulkan(attachment.Format),
            .samples = SamplesToVulkan(attachment.Samples),
            .loadOp = LoadOpToVulkan(attachment.LoadOp),
            .storeOp = StoreOpToVulkan(attachment.StoreOp),
            .stencilLoadOp = LoadOpToVulkan(attachment.StencilLoadOp),
            .stencilStoreOp = StoreOpToVulkan(attachment.StencilStoreOp),
            .initialLayout = ImageLayoutToVulkan(attachment.InitialLayout),
            .finalLayout = ImageLayoutToVulkan(attachment.FinalLayout),
        };

        attachments.push_back(info);
    }

    std::vector<VkSubpassDependency> dependencies;
    std::vector<VkSubpassDescription> subpasses;

    // @note This has to live over here and not in the loop because the
    // destructor will free the underlying memory and the attachment reference
    // will contain garbage.
    std::vector<std::vector<VkAttachmentReference>> colors, inputs, resolves;
    colors.resize(spec.SubPasses.size());
    inputs.resize(spec.SubPasses.size());
    resolves.resize(spec.SubPasses.size());

    VkPipelineStageFlags src_stage_mask{}, dst_stage_mask{};
    VkAccessFlags src_access_mask{}, dst_access_mask{};
    for (u32 i = 0; i < spec.SubPasses.size(); i++) {
        auto& subpass = spec.SubPasses[i];

        for (const auto& color : subpass.ColorAttachments) {
            colors[i].push_back(VkAttachmentReference{
                .attachment = color.Attachment,
                .layout = ImageLayoutToVulkan(color.Layout),
            });

            switch (spec.Attachments[color.Attachment].FinalLayout) {
            case ImageLayout::PresentSrc:
            case ImageLayout::ColorAttachmentOptimal:
                src_stage_mask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dst_stage_mask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }

            if (color.Layout == ImageLayout::ColorAttachmentOptimal) {
                dst_access_mask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            } else {
                dst_access_mask |= VK_ACCESS_SHADER_READ_BIT;
            }
        }

        for (const auto& resolve : subpass.ResolveAttachments) {
            if (resolve.Layout == ImageLayout::Undefined) {
                PANIC("FUCK");
            }

            resolves[i].push_back(VkAttachmentReference{
                .attachment = resolve.Attachment,
                .layout = ImageLayoutToVulkan(resolve.Layout),
            });
        }

        for (const auto& input : subpass.InputAttachments) {
            inputs[i].push_back(VkAttachmentReference{
                .attachment = input.Attachment,
                .layout = ImageLayoutToVulkan(input.Layout),
            });

            if (spec.Attachments[input.Attachment].FinalLayout == ImageLayout::ColorAttachmentOptimal) {
                dst_stage_mask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                if (input.Layout == ImageLayout::ShaderReadOnlyOptimal) {
                    dst_access_mask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                }
            }
        }

        VkAttachmentReference depth_ref{};
        if (subpass.DepthStencilAttachment) {
            depth_ref = VkAttachmentReference{
                .attachment = subpass.DepthStencilAttachment.value().Attachment,
                .layout = ImageLayoutToVulkan(subpass.DepthStencilAttachment.value().Layout),
            };

            if (spec.Attachments[depth_ref.attachment].FinalLayout == ImageLayout::DepthStencilReadOnlyOptimal) {
                dst_access_mask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            }
        }

        auto vk_subpass = VkSubpassDescription{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = CAST(u32, inputs[i].size()),
            .pInputAttachments = inputs[i].data(),
            .colorAttachmentCount = CAST(u32, colors[i].size()),
            .pColorAttachments = colors[i].data(),
            .pResolveAttachments = resolves[i].data(),
            .pDepthStencilAttachment = &depth_ref,
        };

        auto dep = VkSubpassDependency{
            .srcSubpass = i > 0 ? i - 1 : VK_SUBPASS_EXTERNAL,
            .dstSubpass = i,
            .srcStageMask = src_stage_mask,
            .dstStageMask = dst_stage_mask,
            .srcAccessMask = src_access_mask,
            .dstAccessMask = dst_access_mask,
        };

        subpasses.push_back(vk_subpass);
        // dependencies.push_back(dep);
    }

    bool is_shadow_map{ false };
    if (is_shadow_map) {
        dependencies.push_back(VkSubpassDependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        });

        dependencies.push_back(VkSubpassDependency{
            .srcSubpass = 0,
            .dstSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        });
    } else {
        // Depth
        dependencies.push_back(VkSubpassDependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        });

        // Color
        dependencies.push_back(VkSubpassDependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = {},
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        });
    }

    auto renderpass_create_info = VkRenderPassCreateInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = CAST(u32, attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = CAST(u32, subpasses.size()),
        .pSubpasses = subpasses.data(),
        .dependencyCount = CAST(u32, dependencies.size()),
        .pDependencies = dependencies.data(),
    };

    VK_CHECK(vkCreateRenderPass(device->Handle, &renderpass_create_info, nullptr, &m_Handle))

    device->SetHandleName(TRANSMUTE(u64, m_Handle), VK_OBJECT_TYPE_RENDER_PASS, spec.Label);
}

void VulkanRenderPass::Begin() {}

void VulkanRenderPass::End() {}

RenderPassSpecification VulkanRenderPass::GetSpec()
{
    return m_Specification;
}
}
