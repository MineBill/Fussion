#pragma once
#include "CommandBuffer.h"
#include "Fussion/Math/Color.h"

namespace Fussion::RHI {

struct PointLightData {
    Vector3 Position;
    Color Color;
    f32 Radius;
};

struct DirectionalLightData {
    Vector4 Direction;
    Color Color;

    std::array<Mat4, 4> LightSpaceMatrix{};
};

enum class RenderState {
    None = 1 << 0,
    LightCollection = 1 << 1,
    Mesh = 1 << 2,
    Depth = 1 << 3,
};

DECLARE_FLAGS(RenderState, RenderStateFlags)

DECLARE_OPERATORS_FOR_FLAGS(RenderStateFlags)

struct RenderContext {
    Ref<CommandBuffer> Cmd;
    Ref<RenderPass> CurrentPass;
    Ref<Shader> CurrentShader;
    RenderStateFlags RenderFlags;

    std::vector<PointLightData> PointLights{};
    std::vector<DirectionalLightData> DirectionalLights{};
    Mat4 CurrentLightSpace;
};
}
