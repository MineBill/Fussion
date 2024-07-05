#pragma once
#include "CommandBuffer.h"

namespace Fussion {
struct RenderContext {
    Ref<CommandBuffer> Cmd;
    Ref<RenderPass> CurrentPass;
    Ref<Shader> CurrentShader;
};
}
