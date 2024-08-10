#pragma once
#include "VulkanDevice.h"
#include "VulkanPipepline.h"
#include "VulkanRenderPass.h"
#include "Fussion/RHI/Shader.h"
#include "Fussion/Core/Types.h"

namespace Fussion::RHI {
class VulkanShader : public Shader {
public:
    static Ref<VulkanShader> Create(
        VulkanDevice* device,
        Ref<RenderPass> render_pass,
        std::span<RHI::ShaderStage> stages,
        RHI::ShaderMetadata metadata);

    Ref<Pipeline> GetPipeline() const override { return m_Pipeline; }
    RHI::ShaderMetadata const& GetMetadata() override { return m_Metadata; }
    void* GetRawHandle() override { return m_Pipeline->GetRawHandle(); }

    VkShaderModule GetVertexModule() const { return m_VertexModule; }
    VkShaderModule GetFragmentModule() const { return m_FragmentModule; }

private:
    RHI::ShaderMetadata m_Metadata;
    VkShaderModule m_VertexModule{}, m_FragmentModule{};
    Ref<VulkanPipeline> m_Pipeline;
    Ref<VulkanRenderPass> m_RenderPass;
};
}
