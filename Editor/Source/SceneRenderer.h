#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Renderer/CommandBuffer.h"
#include "Fussion/Renderer/FrameBuffer.h"
#include "Fussion/Renderer/RenderPass.h"
#include "Fussion/Renderer/UniformBuffer.h"
#include "Fussion/Scene/Scene.h"
#include "Fussion/Assets/AssetRef.h"
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
    Vector4 AmbientColor{};
};

// == == //

struct RenderCamera {
    Mat4 Perspective, View;
    Vector3 Position;
};

struct RenderPacket {
    RenderCamera Camera;
    Fussion::Scene* Scene = nullptr;
};

class SceneRenderer {
public:
    void Init();
    void Resize(Vector2 new_size);

    void Render(const Ref<Fussion::CommandBuffer>& cmd, RenderPacket const& packet);

    [[nodiscard]]
    Ref<Fussion::FrameBuffer> const& GetFrameBuffer() const { return m_FrameBuffer; }

private:
    // static SceneRenderer* s_Instance;

    Ref<Fussion::Shader> m_PbrShader{}, m_GridShader;
    Ref<Fussion::ResourcePool> m_ResourcePool{};
    Ref<Fussion::Resource> m_GlobalResource{};
    Ref<Fussion::Texture2D> m_TestTexture;

    Fussion::UniformBuffer<ViewData> m_ViewData;
    Fussion::UniformBuffer<DebugOptions> m_DebugOptions;
    Fussion::UniformBuffer<GlobalData> m_GlobalData;

    Fussion::UniformBuffer<SceneData> m_SceneData;
    Ref<Fussion::Resource> m_SceneResource;

    Vector2 m_RenderArea{};
    Ref<Fussion::RenderPass> m_SceneRenderPass{};
    Ref<Fussion::FrameBuffer> m_FrameBuffer{};

    Fussion::RenderContext m_RenderContext;
};
