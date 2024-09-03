#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Types.h"

#include "Fussion/Scene/Scene.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Assets/ShaderAsset.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector4.h"
#include "Fussion/Rendering/UniformBuffer.h"

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
#if 0
    [[nodiscard]]
    Ref<RHI::FrameBuffer> const& GetFrameBuffer() const { return m_FrameBuffers[(m_CurrentFrame - 1) % RHI::MAX_FRAMES_IN_FLIGHT]; }

    [[nodiscard]]
    auto GetDepthImage() const -> Ref<RHI::Image> { return m_DepthImage; }

    [[nodiscard]]
    auto GetObjectPickingFrameBuffer() const -> Ref<RHI::FrameBuffer> const& { return m_ObjectPickingFrameBuffer; }

    [[nodiscard]]
    auto GetShadowDebugFrameBuffer() const -> Ref<RHI::FrameBuffer> const& { return m_ShadowViewerFrameBuffers[0]; }
#endif

private:
    struct InstanceData {
        Mat4 Model;
    };

    void CreateSceneRenderTarget(Vector2 const& size);

    Fussion::GPU::Texture m_SceneRenderTarget{};
    Fussion::GPU::Texture m_SceneRenderDepthTarget{};

    Fussion::GPU::RenderPipeline m_SimpleRp{}, m_GridRp{}, m_PbrRp{};

    Fussion::GPU::BindGroup m_GlobalBindGroup{}, m_SceneBindGroup{};
    Fussion::GPU::BindGroupLayout m_GlobalBindGroupLayout{}, m_SceneBindGroupLayout{}, m_ObjectBindGroupLayout{};

    Fussion::GPU::Buffer m_PbrInstanceBuffer{};
    std::vector<u8> m_PbrInstanceStagingBuffer{};

    Fussion::GPU::Sampler m_LinearSampler{};

    Vector2 m_RenderArea{};

    Fsn::RenderContext m_RenderContext{};
#if 0
    Fsn::AssetRef<Fsn::ShaderAsset> m_PbrShader{}, m_GridShader, m_DepthShader, m_ObjectPickingShader;
    Fsn::AssetRef<Fsn::ShaderAsset> m_SkyShader{};

    std::array<Ref<RHI::ResourcePool>, RHI::MAX_FRAMES_IN_FLIGHT> m_ResourcePool{};
    std::array<Ref<RHI::Resource>, RHI::MAX_FRAMES_IN_FLIGHT> m_GlobalResource{};
    std::array<Ref<RHI::Resource>, RHI::MAX_FRAMES_IN_FLIGHT> m_SceneResource;

    std::array<Ref<RHI::Buffer>, RHI::MAX_FRAMES_IN_FLIGHT> m_InstanceBuffers;
    std::array<Ref<RHI::Buffer>, RHI::MAX_FRAMES_IN_FLIGHT> m_PBRInstanceBuffers{};

    RHI::FrameAllocator m_FrameAllocator;
    std::array<RHI::ResourceUsage, 1> m_ObjectDepthResourceUsages{};
    std::array<RHI::ResourceUsage, 7> m_PBRResourceUsages{};

    Ref<RHI::RenderPass> m_SceneRenderPass{};
    std::array<Ref<RHI::FrameBuffer>, RHI::MAX_FRAMES_IN_FLIGHT> m_FrameBuffers{};

    Ref<RHI::RenderPass> m_DepthPass{};
    Ref<RHI::Image> m_DepthImage{};

    Ref<RHI::RenderPass> m_ObjectPickingRenderPass{};
    Ref<RHI::FrameBuffer> m_ObjectPickingFrameBuffer{};

    std::array<Ref<RHI::FrameBuffer>, ShadowCascades> m_ShadowFrameBuffers{};

    Ref<RHI::RenderPass> m_ShadowPassDebugRenderPass{};
    Fsn::AssetRef<Fsn::ShaderAsset> m_DepthViewerShader{};
    std::array<Ref<RHI::FrameBuffer>, RHI::MAX_FRAMES_IN_FLIGHT> m_ShadowViewerFrameBuffers{};

    u32 m_CurrentFrame{ 0 };
#endif
};
