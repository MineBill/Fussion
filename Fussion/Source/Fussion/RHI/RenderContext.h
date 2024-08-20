#pragma once
#include "CommandBuffer.h"
#include "Fussion/Math/Color.h"

namespace Fussion::RHI {

    struct PointLightData {
        Vector3 Position;
        Color Color;
        f32 Radius;
    };
    static_assert(sizeof(PointLightData) == 32, "PointLightData needs to be kept in sync with the shader equivalent");

    struct DirectionalLight {
        struct ShaderStruct {
            Vector4 Direction{};
            Color Color{};
            std::array<Mat4, 4> LightSpaceMatrix{};
        } ShaderData;

        f32 Split{};
    };
    // static_assert(sizeof(DirectionalLightData) == 288, "DirectionalLightData needs to be kept in sync with the shader equivalent");

    enum class RenderState {
        None = 1 << 0,
        LightCollection = 1 << 1,
        Mesh = 1 << 2,
        Depth = 1 << 3,
        ObjectPicking = 1 << 4,
    };

    DECLARE_FLAGS(RenderState, RenderStateFlags)

    DECLARE_OPERATORS_FOR_FLAGS(RenderStateFlags)

    struct RenderContext {
        Ref<CommandBuffer> Cmd;
        Ref<RenderPass> CurrentPass;
        Ref<Shader> CurrentShader;
        RenderStateFlags RenderFlags;

        std::vector<PointLightData> PointLights{};
        std::vector<DirectionalLight> DirectionalLights{};
        Mat4 CurrentLightSpace;
    };
}
