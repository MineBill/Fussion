#pragma once
#include "Engin5/Core/Types.h"
#include "Engin5/Renderer/CommandBuffer.h"
#include "Engin5/Renderer/FrameBuffer.h"
#include "Engin5/Renderer/RenderPass.h"
#include "Engin5/Renderer/UniformBuffer.h"

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

    void Render(const Ref<Engin5::CommandBuffer>& cmd, const RenderPacket& packet);

    Ref<Engin5::FrameBuffer> const& GetFrameBuffer() const { return m_FrameBuffer; }

private:
    // static SceneRenderer* s_Instance;

    Ref<Engin5::Shader> m_TriangleShader{};
    Ref<Engin5::ResourcePool> m_ResourcePool{};
    Ref<Engin5::Resource> m_GlobalResource{};
    Engin5::UniformBuffer<GlobalData> m_GlobalData;

    Vector2 m_RenderArea{};
    Ref<Engin5::RenderPass> m_SceneRenderPass{};
    Ref<Engin5::FrameBuffer> m_FrameBuffer{};
};
