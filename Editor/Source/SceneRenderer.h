#pragma once
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Assets/ShaderAsset.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector4.h"
#include "Fussion/Rendering/Pipelines/CubeMapSkybox.h"
#include "Fussion/Rendering/Pipelines/IBLIrradiance.h"
#include "Fussion/Rendering/Pipelines/SSAOBlur.h"
#include "Fussion/Rendering/Pipelines/TonemappingPipeline.h"
#include "Fussion/Rendering/UniformBuffer.h"
#include "Fussion/Scene/Scene.h"

namespace Fsn = Fussion;

// == Global Set == //

struct ViewData {
    Mat4 perspective {};
    Mat4 view {};
    Mat4 view_rotation_only {};
    Vector4 position {};
    Vector2 screen_size {}, _padding;
};

struct DebugOptions {
    b32 show_cascade_boxes;
    b32 show_cascade_colors;
};

struct GlobalData {
    f32 time {};
};

// == Scene Set == //

struct SceneData {
    Vector4 view_position {};
    Vector4 view_direction {};
    Color ambient_color {};
};

struct LightData {
    Fsn::GPUDirectionalLight::ShaderStruct directional_light {};

    Vector4 shadow_split_distances {};
};

// == == //

struct RenderCamera {
    Mat4 perspective, view;
    Mat4 rotation;
    Vector3 position;
    f32 near, far;
    Vector3 direction;
};

struct RenderPacket {
    RenderCamera camera;
    Fsn::Scene* scene = nullptr;
    Vector2 size {};
};

constexpr s32 MAX_SHADOW_CASCADES = 4;
constexpr s32 SHADOWMAP_RESOLUTION = 4096;

struct GBuffer {
    Fussion::GPU::Texture PositionRT;
    Fussion::GPU::Texture NormalRT;
    Fussion::GPU::Texture AlbedoRT;

    Fussion::AssetRef<Fussion::ShaderAsset> Shader {};

    void Init(Vector2 const& size);
    void Resize(Vector2 const& new_size);
    void Render(Fussion::GPU::CommandEncoder& encoder);
};

struct SSAO {
    Fussion::GPU::Texture RenderTarget {};
    Fussion::GPU::Texture NoiseTexture {};
    Fussion::AssetRef<Fussion::ShaderAsset> Shader {};
    Fussion::GPU::BindGroup BindGroup {};
    Fussion::GPU::Sampler Sampler {}, NoiseSampler {};

    Fussion::GPU::Buffer SamplesBuffer {};

    Fussion::UniformBuffer<Fussion::PostProcessing::SSAO> Options {};

    void Init(Vector2 const& size, GBuffer const& gbuffer);
    void Resize(Vector2 const& new_size, GBuffer const& gbuffer);

    void UpdateBindGroup(GBuffer const& gbuffer);
};

class SceneRenderer {
public:
    struct RenderDebugOptions {
        s32 CascadeIndex { 0 };
    } RenderDebugOptions;

    struct Timings {
        f64 Depth {};    // [0, 1]
        f64 Gbuffer {};  // [2, 3]
        f64 SSAO {};     // [4, 5]
        f64 SSAOBlur {}; // [6, 7]
        f64 PBR {};      // [8, 9]
    } Timings {};

    struct PipelineStatistics {
        struct Statistic {
            u64 VertexShaderInvocations {};
            u64 ClipperInvocations {};
            u64 FragmentShaderInvocations {};
        };

        Statistic GbufferStats {};
        Statistic SSAOStats {};
        Statistic PBRStats {};
    } PipelineStatistics {};

    Fussion::UniformBuffer<ViewData> SceneViewData;
    Fussion::UniformBuffer<LightData> SceneLightData;

    Fussion::UniformBuffer<DebugOptions> SceneDebugOptions;
    Fussion::UniformBuffer<GlobalData> SceneGlobalData;

    Fussion::UniformBuffer<SceneData> SceneSceneData;

    void Init();
    void Resize(Vector2 const& newSize);

    void Render(Fussion::GPU::CommandEncoder& encoder, RenderPacket const& packet, bool game_view = false);

    auto GetRenderTarget() const -> Fussion::GPU::Texture const& { return m_SceneRenderTarget; }

    GBuffer gbuffer;
    SSAO ssao;
    Fussion::SSAOBlur ssao_blur {};

private:
    struct InstanceData {
        Mat4 Model;
    };

    struct DepthInstanceData {
        Mat4 Model;
        Mat4 LightSpace;
    };

    void SetupSceneBindGroup();
    void UpdateSceneBindGroup(Fussion::GPU::Texture const& ssao_texture);

    void SetupShadowPassRenderTarget();
    void SetupShadowPass();
    void DepthPass(Fussion::GPU::CommandEncoder& encoder, RenderPacket const& packet);
    void PBRPass(Fussion::GPU::CommandEncoder const& encoder, RenderPacket const& packet, bool game_view);
    void SetupQueries();

    void CreateSceneRenderTarget(Vector2 const& size);

    Fussion::TonemappingPipeline m_TonemappingPipeline {};
    Fussion::CubeSkybox m_CubeSkybox {};
    std::map<Fussion::AssetHandle, Fussion::GPU::Texture> m_EnvironmentMaps {};

    Fussion::GPU::Texture m_SceneRenderTarget {};
    Fussion::GPU::Texture m_SceneRenderDepthTarget {};

    Fussion::GPU::Texture m_ShadowPassRenderTarget {};
    std::array<Fussion::GPU::TextureView, MAX_SHADOW_CASCADES> m_ShadowPassRenderTargetViews {};

    Fussion::AssetRef<Fussion::ShaderAsset> m_GridShader {}, m_SkyShader {}, m_DepthShader {};
    Fussion::GPU::RenderPipeline m_GridPipeline {}, m_PBRPipeline {}, m_DebugPipeline {};

    Fussion::GPU::BindGroup m_GlobalBindGroup {}, m_SceneBindGroup {};
    Fussion::GPU::BindGroupLayout m_GlobalBindGroupLayout {}, m_SceneBindGroupLayout {}, m_ObjectBindGroupLayout {};

    std::vector<Fussion::GPU::Buffer> m_InstanceBufferPool {};

    Fussion::GPU::Buffer m_PBRInstanceBuffer {}, m_DepthInstanceBuffer {};
    std::vector<u8> m_PBRInstanceStagingBuffer {};
    std::vector<u8> m_DepthInstanceStagingBuffer {};

    Fussion::GPU::Sampler m_LinearSampler {};
    Fussion::GPU::Sampler m_ShadowSampler {};

    Fussion::GPU::QuerySet m_TimingsSet {};
    /// Used to resolve the query set into it.
    Fussion::GPU::Buffer m_TimingsResolveBuffer {};
    /// Used to read from it on the CPU once we copy the resolve buffer into it.
    Fussion::GPU::Buffer m_TimingsReadBuffer {};

    Fussion::GPU::QuerySet m_StatisticsQuerySet {};
    /// Used to resolve the query set into it.
    Fussion::GPU::Buffer m_StatisticsResolveBuffer {};
    /// Used to read from it on the CPU once we copy the resolve buffer into it.
    Fussion::GPU::Buffer m_StatisticsReadBuffer {};

    Vector2 m_RenderArea {};

    Fsn::RenderContext m_RenderContext {};

    std::vector<Fussion::GPU::BindGroup> m_ObjectGroupsToRelease {};
};
