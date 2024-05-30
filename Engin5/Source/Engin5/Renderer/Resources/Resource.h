#pragma once
#include "ResourcePool.h"
#include "Engin5/Renderer/RenderHandle.h"
#include "Engin5/Renderer/Shader.h"

namespace Engin5
{
    struct ResourceUsage
    {
        std::string Label;
        ResourceType Type;
        s32 Count;
        ShaderTypeFlags Stages;
    };

    class ResourceLayout: public RenderHandle
    {
    };

    class Resource: public RenderHandle
    {
    };
}