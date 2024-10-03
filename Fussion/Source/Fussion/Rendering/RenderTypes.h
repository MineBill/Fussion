#pragma once
#include <Fussion/Core/BitFlags.h>
#include <Fussion/GPU/GPU.h>
#include <Fussion/Math/Color.h>

namespace Fussion {
    class Texture2D;
    class PbrMaterial;
}

namespace Fussion {

    struct GPUSpotLight { };

    static_assert(sizeof(GPUSpotLight) == 1, "GPUSpotLight needs to be kept in sync with the shader equivalent");

    struct GPUPointLight {
        Vector3 Position;
        Color LightColor;
        f32 Radius;
    };

    static_assert(sizeof(GPUPointLight) == 32, "GPUPointLight needs to be kept in sync with the shader equivalent");

    struct GPUDirectionalLight {
        struct ShaderStruct {
            std::array<Mat4, 4> LightSpaceMatrix {};
            Vector4 Direction {};
            Color LightColor {};
            f32 Brightness {};
            f32 __padding[3] {};
        } ShaderData;

        f32 Split {};
    };

    static_assert(sizeof(GPUDirectionalLight) == 308, "GPUDirectionalLight needs to be kept in sync with the shader equivalent");
    static_assert(sizeof(GPUDirectionalLight::ShaderStruct) % 16 == 0, "GPUDirectionalLight needs to be kept in sync with the shader equivalent");

    enum class RenderState {
        None = 1 << 0,
        LightCollection = 1 << 1,
        Mesh = 1 << 2,
        Depth = 1 << 3,
        ObjectPicking = 1 << 4,
    };

    BITFLAGS(RenderState)

    enum class DrawPass : u32 {
        None = 1 << 0,
        Depth = 1 << 2,
        PBR = 1 << 3,

        All = Depth | PBR,
    };

    DECLARE_FLAGS(DrawPass, DrawPassFlags)

    DECLARE_OPERATORS_FOR_FLAGS(DrawPassFlags)

    struct PostProcessing {
        struct SSAO {
            // Unused for now.
            u32 KernelSize { 64 };
            f32 Radius { 0.125f };
            f32 Bias { 0.025f };
            f32 NoiseScale { 4.0f };
        };

        struct Tonemapping {
            f32 Gamma { 2.2f };
            f32 Exposure { 1.0f };
            /// 0 nothing, 1 aces, 2 reinhard
            u32 Mode { 0 };
        } TonemappingSettings {};

        bool UseSSAO {};
        SSAO SSAOData {};
    };

    // NOTE: Ideally this would hold an actual material, that defines
    //       a shader to use. For now, we assume that all render objects
    //       are for the PBR pass.
    struct RenderObject {
        Vector3 Position {};
        Mat4 WorldMatrix {};

        DrawPassFlags Pass;
        PbrMaterial* Material {};
        GPU::Buffer VertexBuffer {};
        GPU::Buffer IndexBuffer {};
        GPU::Buffer InstanceBuffer {};
        u32 IndexCount {};
    };

    using MeshBatchMap = std::unordered_map<GPU::HandleT, std::vector<size_t>>;

    struct RenderContext {
        RenderStateFlags RenderFlags;
        PostProcessing PostProcessingSettings {};
        Texture2D* EnvironmentMap { nullptr };

        std::vector<GPUPointLight> PointLights {};
        std::vector<GPUDirectionalLight> DirectionalLights {};
        Mat4 CurrentLightSpace;

        std::vector<RenderObject> RenderObjects {};

        std::unordered_map<PbrMaterial*, MeshBatchMap> MeshRenderLists {};

        void AddRenderObject(RenderObject const& obj);
        void Reset();
    };
}
