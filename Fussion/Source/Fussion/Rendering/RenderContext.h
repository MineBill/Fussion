#pragma once
#include <Fussion/Math/Color.h>
#include <Fussion/RHI/CommandBuffer.h>

namespace Fussion {

    struct GPUSpotLight {};

    static_assert(sizeof(GPUSpotLight) == 1, "GPUSpotLight needs to be kept in sync with the shader equivalent");

    struct GPUPointLight {
        Vector3 Position;
        Color Color;
        f32 Radius;
    };

    static_assert(sizeof(GPUPointLight) == 32, "GPUPointLight needs to be kept in sync with the shader equivalent");

    struct GPUDirectionalLight {
        struct ShaderStruct {
            Vector4 Direction{};
            Color Color{};
            std::array<Mat4, 4> LightSpaceMatrix{};
        } ShaderData;

        f32 Split{};
    };

    static_assert(sizeof(GPUDirectionalLight) == 292, "GPUDirectionalLight needs to be kept in sync with the shader equivalent");

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
        Ref<RHI::CommandBuffer> Cmd;
        Ref<RHI::RenderPass> CurrentPass;
        Ref<RHI::Shader> CurrentShader;
        RenderStateFlags RenderFlags;

        std::vector<GPUPointLight> PointLights{};
        std::vector<GPUDirectionalLight> DirectionalLights{};
        Mat4 CurrentLightSpace;
    };
}
