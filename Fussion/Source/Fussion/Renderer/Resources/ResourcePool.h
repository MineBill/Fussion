#pragma once
#include "Fussion/Core/Result.h"
#include "Fussion/Renderer/RenderHandle.h"

namespace Fussion
{
    class Resource;
    class ResourceLayout;

    enum class ResourceType
    {
        CombinedImageSampler,
        InputAttachment,
        UniformBuffer,
        StorageBuffer,
        Sampler,
    };

    struct ResourceLimit
    {
        ResourceType Type;
        s32 Limit;
    };

    struct ResourcePoolSpecification
    {
        s32 MaxSets;
        std::vector<ResourceLimit> ResourceLimits;

        static ResourcePoolSpecification Default(s32 max_sets)
        {
            return ResourcePoolSpecification {
                .MaxSets = max_sets,
                .ResourceLimits = {
                    { ResourceType::Sampler, max_sets },
                    { ResourceType::CombinedImageSampler, max_sets },
                    { ResourceType::UniformBuffer, max_sets },
                    { ResourceType::StorageBuffer, max_sets },
                    { ResourceType::InputAttachment, max_sets },
                },
            };
        }
    };

    class ResourcePool: public RenderHandle
    {
    public:
        enum class AllocationError
        {
            OutOfMemory,
            FragmentedPool,
        };

        virtual const ResourcePoolSpecification& GetSpec() = 0;

        virtual auto Allocate(Ref<ResourceLayout> layout, const std::string& name = "") -> Result<Ref<Resource>, AllocationError>  = 0;

        virtual void Reset() = 0;
    };
}