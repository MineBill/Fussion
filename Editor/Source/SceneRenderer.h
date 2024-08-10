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

// == Global Set == //

struct ViewData {
    Mat4 Perspective{};
    Mat4 View{};
    Mat4 RotationView{};
    Vector2 ScreenSize{};
};

struct DebugOptions {
    bool ShowCascadeBoxes;
    bool ShowCascadeColors;
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
    Fussion::RHI::DirectionalLightData DirectionalLight{};

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
    Fussion::Scene* Scene = nullptr;
    Vector2 Size{};
};

class SceneRenderer {
public:
    void Init();
    void Resize(Vector2 const& new_size);

    void Render(Ref<Fussion::RHI::CommandBuffer> const& cmd, RenderPacket const& packet);

    [[nodiscard]]
    Ref<Fussion::RHI::FrameBuffer> const& GetFrameBuffer() const { return m_FrameBuffer; }

    [[nodiscard]]
    auto GetShadowFrameBuffers() const -> std::array<Ref<Fussion::RHI::FrameBuffer>, 4> { return m_ShadowFrameBuffers; }

    [[nodiscard]]
    auto GetDepthImage() const -> Ref<Fussion::RHI::Image> { return m_DepthImage; }

    [[nodiscard]]
    auto GetObjectPickingFrameBuffer() const -> Ref<Fussion::RHI::FrameBuffer> const& { return m_ObjectPickingFrameBuffer; }

private:
    Fussion::AssetRef<Fussion::ShaderAsset> m_PbrShader{}, m_GridShader, m_DepthShader, m_ObjectPickingShader;

    Ref<Fussion::RHI::ResourcePool> m_ResourcePool{};
    Ref<Fussion::RHI::Resource> m_GlobalResource{};
    Ref<Fussion::Texture2D> m_TestTexture;

    Fussion::RHI::UniformBuffer<ViewData> m_ViewData;
    Fussion::RHI::UniformBuffer<DebugOptions> m_DebugOptions;
    Fussion::RHI::UniformBuffer<GlobalData> m_GlobalData;

    Fussion::RHI::UniformBuffer<SceneData> m_SceneData;
    Fussion::RHI::UniformBuffer<LightData> m_LightData;
    Ref<Fussion::RHI::Resource> m_SceneResource;

    Vector2 m_RenderArea{};
    Ref<Fussion::RHI::RenderPass> m_SceneRenderPass{};
    Ref<Fussion::RHI::FrameBuffer> m_FrameBuffer{};

    Ref<Fussion::RHI::RenderPass> m_DepthPass{};
    Ref<Fussion::RHI::Image> m_DepthImage{};

    Ref<Fussion::RHI::RenderPass> m_ObjectPickingRenderPass{};
    Ref<Fussion::RHI::FrameBuffer> m_ObjectPickingFrameBuffer{};

    // Ref<Fussion::ShaderAsset> m_SkyShader{};
    Fussion::AssetRef<Fussion::ShaderAsset> m_SkyShader{};

    std::array<Ref<Fussion::RHI::FrameBuffer>, 4> m_ShadowFrameBuffers{};

    Fussion::RHI::RenderContext m_RenderContext;
};
