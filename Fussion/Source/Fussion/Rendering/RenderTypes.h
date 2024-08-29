#pragma once
#include <Fussion/Math/Color.h>
#include <Fussion/RHI/CommandBuffer.h>
#include <magic_enum/magic_enum.hpp>

namespace Fussion {
    class PbrMaterial;
}

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

    enum class DrawPass: u32 {
        None = 1 << 0,
        Depth = 1 << 2,
        PBR = 1 << 3,

        All = Depth | PBR,
    };

    constexpr auto DrawPassCount = magic_enum::enum_count<DrawPass>();

    DECLARE_FLAGS(DrawPass, DrawPassFlags)

    DECLARE_OPERATORS_FOR_FLAGS(DrawPassFlags)

    // NOTE: Ideally this would hold an actual material, that defines
    //       a shader to use. For now, we assume that all render objects
    //       are for the PBR pass.
    struct RenderObject {
        Mat4 WorldMatrix{};

        DrawPassFlags Pass;
        PbrMaterial* Material{};
        Ref<RHI::Buffer> VertexBuffer{};
        Ref<RHI::Buffer> IndexBuffer{};
        u32 IndexCount{};
    };

    struct RenderContext {
        Ref<RHI::CommandBuffer> Cmd;
        Ref<RHI::RenderPass> CurrentPass;
        Ref<RHI::Shader> CurrentShader;
        RenderStateFlags RenderFlags;

        std::vector<GPUPointLight> PointLights{};
        std::vector<GPUDirectionalLight> DirectionalLights{};
        Mat4 CurrentLightSpace;

        std::vector<RenderObject> RenderObjects{};

        void AddRenderObject(RenderObject& obj);
        void Reset();
    };
}
