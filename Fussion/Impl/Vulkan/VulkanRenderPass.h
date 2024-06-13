#pragma once
#include "VulkanDevice.h"
#include "Fussion/Renderer/RenderPass.h"

namespace Fussion
{
    class VulkanRenderPass: public RenderPass
    {
    public:
        VulkanRenderPass(VulkanDevice* device, RenderPassSpecification spec);

        void Begin() override;
        void End() override;

        RenderPassSpecification GetSpec() override;
        void* GetRawHandle() override { return m_Handle; }
    private:
        VkRenderPass m_Handle;

        RenderPassSpecification m_Specification;
    };
}