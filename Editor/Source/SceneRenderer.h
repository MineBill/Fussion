#pragma once
#include "Fussion/Core/Types.h"

#include "Fussion/Scene/Scene.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector4.h"
#include "Fussion/Rendering/UniformBuffer.h"
#include "Fussion/Rendering/Pipelines/HDRPipeline.h"

namespace Fsn = Fussion;

// == Global Set == //

struct ViewData {
    Mat4 Perspective{};
    Mat4 View{};
    Vector4 Position{};

    // Mat4 RotationView{};
    // Vector2 ScreenSize{};
};

struct DebugOptions {
    b32 ShowCascadeBoxes;
    b32 ShowCascadeColors;
};

struct GlobalData {
    f32 Time{};
};

// == Scene Set == //

struct SceneData {
    Vector4 ViewPosition{};
    Vector4 ViewDirection{};
    Color AmbientColor{};
};

struct LightData {
    Fsn::GPUDirectionalLight::ShaderStruct DirectionalLight{};

    Vector4 ShadowSplitDistances{};
};

// == == //

struct RenderCamera {
    Mat4 Perspective, View;
    Vector3 Position;
    f32 Near, Far;
    Vector3 Direction;
};

struct RenderPacket {
    RenderCamera Camera;
    Fsn::Scene* Scene = nullptr;
    Vector2 Size{};
};

constexpr s32 ShadowCascades = 4;
constexpr s32 ShadowMapResolution = 4096;


class SceneRenderer {
public:
    struct RenderDebugOptions {
        s32 CascadeIndex{ 0 };
    } RenderDebugOptions;

    Fussion::UniformBuffer<ViewData> SceneViewData;
    Fussion::UniformBuffer<LightData> SceneLightData;

    Fussion::UniformBuffer<DebugOptions> SceneDebugOptions;
    Fussion::UniformBuffer<GlobalData> SceneGlobalData;

    Fussion::UniformBuffer<SceneData> SceneSceneData;

    void Init();
    void Resize(Vector2 const& new_size);

    void Render(Fussion::GPU::CommandEncoder& encoder, RenderPacket const& packet, bool game_view = false);

    auto GetRenderTarget() const -> Fussion::GPU::Texture const& { return m_SceneRenderTarget; }

private:
    struct InstanceData {
        Mat4 Model;
    };

    struct DepthInstanceData {
        Mat4 Model;
        Mat4 LightSpace;
    };

    void SetupShadowPassRenderTarget();
    void SetupShadowPass();
    void DepthPass(Fussion::GPU::CommandEncoder& encoder, RenderPacket const& packet);
    void PbrPass(Fussion::GPU::CommandEncoder& encoder, RenderPacket const& packet, bool game_view);

    void CreateSceneRenderTarget(Vector2 const& size);

    Fussion::HDRPipeline m_HDRPipeline{};

    Fussion::GPU::Texture m_SceneRenderTarget{};
    Fussion::GPU::Texture m_SceneRenderDepthTarget{};

    Fussion::GPU::Texture m_ShadowPassRenderTarget{};
    std::array<Fussion::GPU::TextureView, ShadowCascades> m_ShadowPassRenderTargetViews{};

    Fussion::GPU::RenderPipeline m_SimplePipeline{}, m_GridPipeline{}, m_PbrPipeline{}, m_DepthPipeline{}, m_SkyPipeline{}, m_DebugPipeline{};

    Fussion::GPU::BindGroup m_GlobalBindGroup{}, m_SceneBindGroup{};
    Fussion::GPU::BindGroupLayout m_GlobalBindGroupLayout{}, m_SceneBindGroupLayout{}, m_ObjectBindGroupLayout{}, m_ObjectDepthBGL{};

    std::vector<Fussion::GPU::Buffer> m_InstanceBufferPool{};

    Fussion::GPU::Buffer m_PbrInstanceBuffer{}, m_DepthInstanceBuffer{};
    std::vector<u8> m_PbrInstanceStagingBuffer{};
    std::vector<u8> m_DepthInstanceStagingBuffer{};

    Fussion::GPU::Sampler m_LinearSampler{};
    Fussion::GPU::Sampler m_ShadowSampler{};

    Vector2 m_RenderArea{};

    Fsn::RenderContext m_RenderContext{};

    std::vector<Fussion::GPU::BindGroup> m_ObjectGroupsToRelease{};
};
