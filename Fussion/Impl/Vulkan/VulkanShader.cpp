#include "e5pch.h"
#include "VulkanShader.h"

#include <magic_enum/magic_enum.hpp>

namespace Fussion {
Ref<VulkanShader> VulkanShader::Create(
    VulkanDevice* device,
    Ref<RenderPass> render_pass,
    std::span<ShaderStage> stages,
    ShaderMetadata metadata)
{
    auto self = MakeRef<VulkanShader>();
    self->m_Metadata = metadata;

    for (const auto& stage : stages) {
        VkShaderModuleCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ci.codeSize = stage.Bytecode.size() * 4;
        ci.pCode = stage.Bytecode.data();

        if (stage.Type == ShaderType::Vertex) {
            vkCreateShaderModule(device->Handle, &ci, nullptr, &self->m_VertexModule);
        } else if (stage.Type == ShaderType::Fragment) {
            vkCreateShaderModule(device->Handle, &ci, nullptr, &self->m_FragmentModule);
        }
    }

    auto spec = PipelineLayoutSpecification{
        .Label = "Pipeline Layout",
        .UsePushDescriptor = false,
        .ResourceUsages = {},
    };

    spec.PushConstants = metadata.PushConstants;

    std::vector<ResourceUsage> resources = {};

    for (const auto& [set, bindings] : metadata.Uniforms) {
        for (const auto& [binding, usage] : bindings) {
            resources.push_back(usage);
        }
    }

    const auto resource_layout = device->CreateResourceLayout(resources);
    const auto layout = device->CreatePipelineLayout({ resource_layout }, spec)->As<VulkanPipelineLayout>();

    const auto pipeline_spec = PipelineSpecification{
        .Label = "",
        .AttributeLayout = VertexAttributeLayout::Create(metadata.VertexAttributes),
        .ShaderStages = stages,
        .UseBlending = metadata.UseBlending,
    };
    self->m_Pipeline = device->CreatePipeline(render_pass, self, layout, pipeline_spec)->As<VulkanPipeline>();

    return self;
}
}
