#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Types.h"
#include "Fussion/RHI/CommandBuffer.h"
#include "Fussion/RHI/FrameBuffer.h"
#include "Fussion/RHI/RenderPass.h"
#include "Fussion/RHI/UniformBuffer.h"
#include "Fussion/Scene/Scene.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Assets/ShaderAsset.h"
#include "Fussion/Math/Vector2.h"
#include "Fussion/Math/Vector4.h"
#include "Fussion/RHI/FrameAllocator.h"

#include <Fussion/RHI/Swapchain.h>

namespace RHI = Fussion::RHI;
namespace Fsn = Fussion;

// == Global Set == //

struct ViewData {
    Mat4 Perspective{};
    Mat4 View{};
    Mat4 RotationView{};
    Vector2 ScreenSize{};
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

    RHI::UniformBuffer<ViewData> SceneViewData;
    RHI::UniformBuffer<DebugOptions> SceneDebugOptions;
    RHI::UniformBuffer<GlobalData> SceneGlobalData;

    RHI::UniformBuffer<SceneData> SceneSceneData;
    RHI::UniformBuffer<LightData> SceneLightData;

    void Init();
    void Resize(Vector2 const& new_size);

    void Render(Ref<RHI::CommandBuffer> const& cmd, RenderPacket const& packet, bool game_view = false);

    [[nodiscard]]
    Ref<RHI::FrameBuffer> const& GetFrameBuffer() const { return m_FrameBuffers[(m_CurrentFrame - 1) % RHI::MAX_FRAMES_IN_FLIGHT]; }

    // [[nodiscard]]
    // auto GetShadowFrameBuffers() const -> std::array<Ref<Fussion::RHI::FrameBuffer>, 4> { return m_ShadowFrameBuffers; }

    [[nodiscard]]
    auto GetDepthImage() const -> Ref<RHI::Image> { return m_DepthImage; }

    [[nodiscard]]
    auto GetObjectPickingFrameBuffer() const -> Ref<RHI::FrameBuffer> const& { return m_ObjectPickingFrameBuffer; }

    [[nodiscard]]
    auto GetShadowDebugFrameBuffer() const -> Ref<RHI::FrameBuffer> const& { return m_ShadowViewerFrameBuffers[0]; }

private:
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

    Vector2 m_RenderArea{};
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

    Fsn::RenderContext m_RenderContext{};
    u32 m_CurrentFrame{ 0 };
};
