#pragma once
#include "VulkanDevice.h"
#include "Fussion/RHI/RenderPass.h"

namespace Fussion::RHI {
class VulkanRenderPass : public RenderPass {
public:
    VulkanRenderPass(VulkanDevice* device, RenderPassSpecification spec);

    virtual void Begin() override;
    virtual void End() override;

    virtual auto GetSpec() -> RenderPassSpecification const& override;
    virtual auto GetRawHandle() -> void* override { return m_Handle; }

private:
    VkRenderPass m_Handle;

    RenderPassSpecification m_Specification;
};
}
