#pragma once
#include "RenderHandle.h"
#include "VertexLayout.h"
#include "Resources/Resource.h"

#include <vulkan/vulkan_core.h>

namespace Fussion {
struct PipelineLayoutSpecification {
    std::string Label;
    bool UsePushDescriptor;
    std::vector<Ref<ResourceLayout>> ResourceUsages{};
    std::vector<PushConstant> PushConstants{};
};

class PipelineLayout : public RenderHandle {
public:
    virtual PipelineLayoutSpecification GetSpec() = 0;
};

struct PipelineSpecification {
    std::string Label{};
    VertexAttributeLayout AttributeLayout{};
    std::span<ShaderStage> ShaderStages{};
    bool UseBlending{};
};

class Pipeline : public RenderHandle {
public:
    virtual PipelineSpecification GetSpec() = 0;
    virtual Ref<PipelineLayout> GetLayout() = 0;
};
}
