#pragma once
#include "VulkanDevice.h"
#include "Fussion/Renderer/Sampler.h"

#include "volk.h"

namespace Fussion
{
    class VulkanSampler: public Sampler
    {
    public:
        VulkanSampler(VulkanDevice* device, SamplerSpecification spec);
        void Destroy() override;

        SamplerSpecification GetSpec() override { return m_Specification; }
        void* GetRawHandle() override { return m_Handle; }

    private:
        SamplerSpecification m_Specification{};
        VkSampler m_Handle{};
    };
}
