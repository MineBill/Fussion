#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Renderer/CommandBuffer.h"
#include "Fussion/Renderer/FrameBuffer.h"
#include "Fussion/Renderer/RenderPass.h"
#include "Fussion/Renderer/UniformBuffer.h"

struct GlobalData
{
    Mat4 Perspective{};
    Mat4 View{};
};

struct RenderCamera
{
    Mat4 Perspective, View;
    Vector3 Position;
};

struct RenderPacket
{
    RenderCamera Camera;
};

class SceneRenderer
{
public:
    void Init();
    void Resize(Vector2 new_size);

    void Render(const Ref<Fussion::CommandBuffer>& cmd, const RenderPacket& packet);

    Ref<Fussion::FrameBuffer> const& GetFrameBuffer() const { return m_FrameBuffer; }

private:
    // static SceneRenderer* s_Instance;

    Ref<Fussion::Shader> m_TriangleShader{};
    Ref<Fussion::ResourcePool> m_ResourcePool{};
    Ref<Fussion::Resource> m_GlobalResource{};
    Fussion::UniformBuffer<GlobalData> m_GlobalData;
    Ref<Fussion::Texture2D> m_TestTexture;

    Vector2 m_RenderArea{};
    Ref<Fussion::RenderPass> m_SceneRenderPass{};
    Ref<Fussion::FrameBuffer> m_FrameBuffer{};
};
