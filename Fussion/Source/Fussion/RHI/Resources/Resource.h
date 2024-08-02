#pragma once
#include "ResourcePool.h"
#include "Fussion/RHI/RenderHandle.h"
#include "Fussion/RHI/ShaderFlags.h"

namespace Fussion::RHI {
struct ResourceUsage {
    std::string Label = "Resource Usage";
    ResourceType Type;
    s32 Count = 1;
    ShaderTypeFlags Stages;
};

class ResourceLayout : public RenderHandle {};

class Resource : public RenderHandle {};
}
