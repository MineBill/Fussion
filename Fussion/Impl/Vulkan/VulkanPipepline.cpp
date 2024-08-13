#include "e5pch.h"
#include "VulkanPipepline.h"

#include <ranges>
#include <utility>

#include "Common.h"
#include "VulkanImage.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "Resources/VulkanResource.h"

namespace Fussion::RHI {
VulkanPipelineConfig VulkanPipelineConfig::Default()
{
    VulkanPipelineConfig ret{};

    ret.InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    // @todo Make this configurable
    ret.InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    ret.InputAssembly.primitiveRestartEnable = VK_FALSE;

    ret.Rasterization = VkPipelineRasterizationStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = false,
        .rasterizerDiscardEnable = false,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = false,
        // .depthBiasConstantFactor = ,
        // .depthBiasClamp = ,
        // .depthBiasSlopeFactor = ,
        .lineWidth = 1,
    };

    ret.Multisample = VkPipelineMultisampleStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = false,
    };

    ret.ColorBlendAttachment = VkPipelineColorBlendAttachmentState{
        .blendEnable = false,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT,
    };

    ret.ColorBlendState = VkPipelineColorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = false,
        .attachmentCount = 1,
        .pAttachments = nullptr,
    };

    ret.DepthStencil = VkPipelineDepthStencilStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = true,
        .depthWriteEnable = true,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = false,
        .stencilTestEnable = false,
    };

    return ret;
}

Ref<VulkanPipelineLayout> VulkanPipelineLayout::Create(
    VulkanDevice* device,
    const std::vector<Ref<ResourceLayout>>& layouts,
    PipelineLayoutSpecification spec)
{
    auto self = MakeRef<VulkanPipelineLayout>();
    self->m_Specification = std::move(spec);

    std::vector<VkDescriptorSetLayout> vk_layouts;
    vk_layouts.reserve(layouts.size());

    for (const auto& layout : layouts) {
        vk_layouts.push_back(layout->GetRenderHandle<VkDescriptorSetLayout>());
    }

    auto ci = VkPipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = CAST(u32, vk_layouts.size()),
        .pSetLayouts = vk_layouts.data(),
    };

    auto push_constant_count = self->m_Specification.PushConstants.size();
    ci.pushConstantRangeCount = CAST(u32, push_constant_count);

    std::vector<VkPushConstantRange> ranges(push_constant_count);
    for (size_t i = 0; i < push_constant_count; i++) {
        ranges[i] = VkPushConstantRange{
            .offset = 0,
            .size = CAST(u32, self->m_Specification.PushConstants[i].Size),
        };

        if (self->m_Specification.PushConstants[i].Stage == ShaderType::Vertex) {
            ranges[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        } else {
            ranges[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        }
    }
    ci.pPushConstantRanges = ranges.data();

    VK_CHECK(vkCreatePipelineLayout(device->Handle, &ci, nullptr, &self->m_Handle));

    return self;
}

VkVertexInputBindingDescription VertexVulkanBindingDescription(VertexAttributeLayout const& layout)
{
    return {
        .binding = 0,
        .stride = CAST(u32, layout.Stride),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

VkFormat VertexElementTypeToVulkan(ElementType type)
{
    switch (type) {
        using enum ElementType;
    case Float:
        return VK_FORMAT_R32_SFLOAT;
    case Int:
        return VK_FORMAT_R32_SINT;
    case Float2:
        return VK_FORMAT_R32G32_SFLOAT;
    case Int2:
        return VK_FORMAT_R32G32_SINT;
    case Float3:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case Int3:
        return VK_FORMAT_R32G32B32_SINT;
    case Float4:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Int4:
        return VK_FORMAT_R32G32B32A32_SINT;
    case Mat3:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case Mat4:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
}

std::vector<VkVertexInputAttributeDescription> VertexVulkanAttributeDescription(VertexAttributeLayout const& layout)
{
    std::vector<VkVertexInputAttributeDescription> ret;
    for (u32 i = 0; i < layout.Attributes.size(); i++) {
        ret.push_back(VkVertexInputAttributeDescription{
            .location = i,
            .binding = 0,
            .format = VertexElementTypeToVulkan(layout.Attributes[i].Type),
            .offset = CAST(u32, layout.Attributes[i].Offset),
        });
    }
    return ret;
}

VkPrimitiveTopology PipelineTopologyToVulkan(PipelineTopology topology)
{
    switch (topology) {
    case PipelineTopology::Triangles:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case PipelineTopology::Lines:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case PipelineTopology::TriangleStrip:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    }
    UNREACHABLE;
}


Ref<VulkanPipeline> VulkanPipeline::Create(
    VulkanDevice* device,
    Ref<Shader> shader,
    const Ref<PipelineLayout>& layout,
    Ref<RenderPass> render_pass,
    PipelineSpecification spec)
{
    auto self = MakeRef<VulkanPipeline>();
    self->m_Specification = spec;
    self->m_Layout = layout->As<VulkanPipelineLayout>();
    auto vk_shader = shader->As<VulkanShader>();

    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_infos{};
    for (const auto& stage : spec.ShaderStages) {
        if (stage.Type == ShaderType::Vertex) {
            auto info = VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vk_shader->GetVertexModule(),
                .pName = "main",
            };
            shader_stage_create_infos.push_back(info);
        } else if (stage.Type == ShaderType::Fragment) {
            auto info = VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = vk_shader->GetFragmentModule(),
                .pName = "main",
            };
            shader_stage_create_infos.push_back(info);
        }
    }

    auto binding_desc = VertexVulkanBindingDescription(spec.AttributeLayout);
    auto attributes_desc = VertexVulkanAttributeDescription(spec.AttributeLayout);
    auto vertex_input_state = VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = !spec.AttributeLayout.Attributes.empty() ? 1_u32 : 0_u32,
        .pVertexBindingDescriptions = &binding_desc,
        .vertexAttributeDescriptionCount = CAST(u32, attributes_desc.size()),
        .pVertexAttributeDescriptions = attributes_desc.data(),
    };

    VkDynamicState states[] = { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };
    auto dynamic_state = VkPipelineDynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = states,
    };

    auto config = VulkanPipelineConfig::Default();
    config.ColorBlendAttachment.blendEnable = spec.UseBlending;
    config.Multisample.rasterizationSamples = SampleCountToVulkan(spec.Samples);

    config.InputAssembly.topology = PipelineTopologyToVulkan(spec.Topology);

    std::vector<VkPipelineColorBlendAttachmentState> blend_attachment_states;
    for (auto rspec = render_pass->GetSpec(); auto const& thing : rspec.SubPasses[0].ColorAttachments) {
        blend_attachment_states.push_back(config.ColorBlendAttachment);
    }

    config.ColorBlendState.attachmentCount = CAST(u32, blend_attachment_states.size());
    config.ColorBlendState.pAttachments = blend_attachment_states.data();

    auto viewport = VkViewport{};
    auto scissor = VkRect2D{};
    auto viewport_ci = VkPipelineViewportStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
    auto pipeline_ci = VkGraphicsPipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stage_create_infos.data(),
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &config.InputAssembly,
        // .pTessellationState = ,
        .pViewportState = &viewport_ci,
        .pRasterizationState = &config.Rasterization,
        .pMultisampleState = &config.Multisample,
        .pDepthStencilState = &config.DepthStencil,
        .pColorBlendState = &config.ColorBlendState,
        .pDynamicState = &dynamic_state,
        .layout = layout->GetRenderHandle<VkPipelineLayout>(),
        .renderPass = render_pass->GetRenderHandle<VkRenderPass>(),
        .subpass = 0_u32,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1,
    };

    VK_CHECK(vkCreateGraphicsPipelines(device->Handle, nullptr, 1, &pipeline_ci, nullptr, &self->m_Handle));

    device->SetHandleName(TRANSMUTE(u64, self->m_Handle), VK_OBJECT_TYPE_PIPELINE, spec.Label);

    return self;
}
}
