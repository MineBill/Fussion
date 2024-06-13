#pragma once
#include "VulkanDevice.h"
#include "VulkanPipepline.h"
#include "Fussion/Renderer/Shader.h"
#include "Fussion/Core/Types.h"

namespace Fussion
{
    class VulkanShader: public Shader
    {
    public:
        static Ref<VulkanShader> Create(
            VulkanDevice *device,
            Ref<RenderPass> render_pass,
            std::span<ShaderStage> stages,
            ShaderMetadata metadata);

        Ref<Pipeline> GetPipeline() const override { return m_Pipeline; }
        ShaderMetadata const& GetMetadata() override { return m_Metadata; }
        void* GetRawHandle() override { return nullptr; }

        VkShaderModule GetVertexModule() const { return m_VertexModule; }
        VkShaderModule GetFragmentModule() const { return m_FragmentModule; }


    private:
        ShaderMetadata m_Metadata;
        VkShaderModule m_VertexModule{}, m_FragmentModule{};
        Ref<VulkanPipeline> m_Pipeline;
    };
}
