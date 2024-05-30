#pragma once
#include "VulkanDevice.h"
#include "Engin5/Renderer/Sampler.h"

#include "volk.h"

namespace Engin5
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
