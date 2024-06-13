#pragma once
#include "VulkanDevice.h"
#include "Fussion/Renderer/Pipeline.h"

namespace Fussion
{
    struct VulkanPipelineConfig
    {
        VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
        VkPipelineRasterizationStateCreateInfo Rasterization{};
        VkPipelineMultisampleStateCreateInfo Multisample{};
        VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo ColorBlendState{};
        VkPipelineDepthStencilStateCreateInfo DepthStencil{};

        static VulkanPipelineConfig Default();
    };

    class VulkanPipelineLayout: public PipelineLayout
    {
    public:
        VulkanPipelineLayout() = default;

        static Ref<VulkanPipelineLayout> Create(
            VulkanDevice *device,
            const std::vector<Ref<ResourceLayout>>& layouts,
            PipelineLayoutSpecification spec);

        PipelineLayoutSpecification GetSpec() override { return m_Specification; }
        void* GetRawHandle() override { return m_Handle; }

    private:
        PipelineLayoutSpecification m_Specification;
        VkPipelineLayout m_Handle{};
    };

    class VulkanPipeline: public Pipeline
    {
    public:
        VulkanPipeline() = default;

        static Ref<VulkanPipeline> Create(
            VulkanDevice *device,
            Ref<Shader> shader,
            const Ref<PipelineLayout>& layout,
            Ref<RenderPass> render_pass,
            PipelineSpecification spec);

        Ref<PipelineLayout> GetLayout() override {return m_Layout;}
        PipelineSpecification GetSpec() override {return m_Specification;}
        void* GetRawHandle() override { return m_Handle; }

    private:
        Ref<VulkanPipelineLayout> m_Layout;
        PipelineSpecification m_Specification{};
        VkPipeline m_Handle{};
    };
}
