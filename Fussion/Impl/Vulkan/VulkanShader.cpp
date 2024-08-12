#include "e5pch.h"
#include "VulkanShader.h"

#include <magic_enum/magic_enum.hpp>
#include <algorithm>

namespace Fussion::RHI {
Ref<VulkanShader> VulkanShader::Create(
    VulkanDevice* device,
    Ref<RenderPass> render_pass,
    std::span<ShaderStage> stages,
    ShaderMetadata metadata)
{
    auto self = MakeRef<VulkanShader>();
    self->m_Metadata = metadata;
    self->m_RenderPass = render_pass->As<VulkanRenderPass>();

    for (auto const& stage : stages) {
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

    std::vector<std::vector<ResourceUsage>> resources = {};

    for (auto const& [set, bindings] : metadata.Uniforms) {
        resources.emplace_back();
        for (auto const& [binding, usage] : bindings) {
            (void)usage;
            resources[set].push_back(usage);
        }
    }

    std::vector<Ref<ResourceLayout>> resource_layouts;
    std::ranges::transform(resources, std::back_inserter(resource_layouts), [&](std::vector<ResourceUsage>& resource_usage) {
        return device->CreateResourceLayout(resource_usage);
    });
    const auto layout = device->CreatePipelineLayout(resource_layouts, spec)->As<VulkanPipelineLayout>();

    auto pipeline_spec = PipelineSpecification{
        .Label = "",
        .AttributeLayout = VertexAttributeLayout::Create(metadata.VertexAttributes),
        .ShaderStages = stages,
        .UseBlending = metadata.UseBlending,
        .Samples = metadata.Samples,
    };

    // for (auto const& a : pipeline_spec.AttributeLayout.Attributes) {
    //     LOG_DEBUGF("VertexAttribute: {} | Type: {}", a.Name, magic_enum::enum_name(a.Type));
    // }

    using namespace std::string_literals;

    if (auto pragma = std::ranges::find_if(metadata.ParsedPragmas, [](ParsedPragma const& pragma) {
        return pragma.Key == "topology";
    }); pragma != metadata.ParsedPragmas.end()) {
        if (pragma->Value == "triangles"s) {
            pipeline_spec.Topology = PipelineTopology::Triangles;
        } else if (pragma->Value == "lines"s) {
            pipeline_spec.Topology = PipelineTopology::Lines;
        } else {
            // Default is triangle list.
            pipeline_spec.Topology = PipelineTopology::Triangles;
        }
    }
    self->m_Pipeline = device->CreatePipeline(render_pass, self, layout, pipeline_spec)->As<VulkanPipeline>();

    return self;
}
}
