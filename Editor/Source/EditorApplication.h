#pragma once
#include "EditorRenderer.h"
#include "Engin5/Core/Application.h"
#include "Engin5/Renderer/UniformBuffer.h"
#include "Layers/ImGuiLayer.h"

struct GlobalData
{
    glm::mat4 Perspective{};
    glm::mat4 View{};
};

class EditorApplication: public Engin5::Application
{
public:
    void OnStart() override;
    void OnUpdate(f32 delta) override;

private:
    Ptr<ImGuiLayer> m_Layer;
    Ref<Engin5::Shader> m_TriangleShader{};
    Ref<Engin5::ResourcePool> m_ResourcePool{};

    Ref<Engin5::RenderPass> m_SceneRenderPass{};
    Ref<Engin5::FrameBuffer> m_FrameBuffer{};

    Ref<Engin5::Resource> m_GlobalResource{};
    Ptr<Engin5::UniformBuffer<GlobalData>> m_GlobalData;

    EditorRenderer m_SceneRenderer{};
    f32 m_FOV{};
    glm::mat4 m_Perspective{};
    glm::mat4 m_View{};

    Vector2 m_ViewportSize{};
};
