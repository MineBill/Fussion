#pragma once
#include "EditorCamera.h"
#include "SceneRenderer.h"
#include "Engin5/Core/Application.h"
#include "Engin5/Renderer/UniformBuffer.h"
#include "Layers/ImGuiLayer.h"

class EditorLayer;
class EditorApplication: public Engin5::Application
{
public:
    void OnStart() override;
    void OnUpdate(f32 delta) override;
    void OnEvent(Engin5::Event&) override;

    void Resize(Vector2 size);

    EditorCamera& GetCamera() { return m_Camera; }

    static EditorApplication* Instance() { return s_EditorInstance; }

    SceneRenderer& GetSceneRenderer() { return m_SceneRenderer; }
private:
    static EditorApplication* s_EditorInstance;
    Ptr<ImGuiLayer> m_ImGuiLayer;

    EditorCamera m_Camera;
    SceneRenderer m_SceneRenderer{};
    f32 m_FOV{50.0f};
    glm::mat4 m_Perspective{};
    glm::mat4 m_View{};

    Vector2 m_ViewportSize{10, 10};
    EditorLayer* m_EditorLayer{};
};
