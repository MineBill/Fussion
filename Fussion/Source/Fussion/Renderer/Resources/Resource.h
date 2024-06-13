#pragma once
#include "ResourcePool.h"
#include "Fussion/Renderer/RenderHandle.h"
#include "Fussion/Renderer/Shader.h"

namespace Fussion
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