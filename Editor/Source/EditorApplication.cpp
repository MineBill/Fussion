#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "EditorApplication.h"
#include "Layers/ImGuiLayer.h"

#include "Engin5/Renderer/Renderer.h"
#include "Engin5/Input/Input.h"
#include "imgui.h"


#include <glm/gtc/matrix_transform.hpp>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "Engin5/Events/ApplicationEvents.h"
#include "Engin5/Log/FileSink.h"
#include "Engin5/OS/FileSystem.h"
#include "Layers/EditorLayer.h"

EditorApplication* EditorApplication::s_EditorInstance;

void EditorApplication::OnStart()
{
    ZoneScoped;
    using namespace Engin5;

    s_EditorInstance = this;
    Log::DefaultLogger()->RegisterSink(FileSink::Create("Pepegas.log"));
    LOG_DEBUGF("Current path is: {}", std::filesystem::current_path().string());

    s_EditorInstance = this;
    m_ImGuiLayer = MakePtr<ImGuiLayer>();
    m_ImGuiLayer->Init();

    m_ImGuiLayer->OnDisable();

    m_SceneRenderer.Init();
    m_Camera.Resize(m_Window->GetSize());

    PushLayer(m_ImGuiLayer.get());
    PushLayer(m_EditorLayer = new EditorLayer());

    Application::OnStart();

    // auto aspect = cast(f32, m_Window->GetWidth()) / cast(f32, m_Window->GetHeight());
    // m_Perspective = glm::perspective(glm::radians(50.0f), aspect, 0.1f, 1000.0f);
}

void EditorApplication::OnUpdate(const f32 delta)
{
    ZoneScoped;
    using namespace Engin5;

    m_ImGuiLayer->Begin();

    m_Camera.SetFocus(m_EditorLayer->GetViewport().IsFocused());
    m_Camera.OnUpdate(delta);
    // Update the layers.
    Application::OnUpdate(delta);

    // auto forward = Input::GetAxis(KeyboardKey::W, KeyboardKey::S);
    // auto strafe = Input::GetAxis(KeyboardKey::A, KeyboardKey::D);
    // m_GlobalData->GetData().View = glm::translate(m_GlobalData->GetData().View, Vector3(strafe, 0, forward) * 0.001f);
    //
    //
    // // const auto aspect = cast(f32, m_Window->GetWidth()) / cast(f32, m_Window->GetHeight());
    // const auto aspect = m_ViewportSize.x / m_ViewportSize.y;
    // m_Perspective = glm::perspective(glm::radians(m_FOV), aspect, 0.1f, 1000.0f);
    // m_GlobalData->GetData().Perspective = m_Perspective;
    // m_GlobalData->Flush();

    auto [cmd, image] = Renderer::Begin();
    if (cmd == nullptr) {
        m_ImGuiLayer->End(cmd);
        return;
    }

    cmd->Begin();
    auto window_size = Vector2{cast(f32, m_Window->GetWidth()), cast(f32, m_Window->GetHeight())};

    m_SceneRenderer.Render(cmd, {
        .Camera = RenderCamera {
            .Perspective = m_Camera.GetPerspective(),
            .View = m_Camera.GetView(),
            .Position = m_Camera.GetPosition(),
        }
    });

    const auto main = Renderer::GetInstance()->GetMainRenderPass();
    cmd->BeginRenderPass(main, Renderer::GetInstance()->GetSwapchain()->GetFrameBuffer(image));
    cmd->SetViewport({window_size.x, -window_size.y});
    cmd->SetScissor({0, 0, window_size.x, window_size.y});

    m_ImGuiLayer->End(cmd);

    cmd->EndRenderPass(main);

    cmd->End();

    Renderer::End(cmd);
}

void EditorApplication::OnEvent(Engin5::Event& event)
{
    using namespace Engin5;
    EventDispatcher dispatcher(event);

    dispatcher.Dispatch<WindowResized>([](WindowResized &resized) -> bool {
        (void)resized;
        // const Vector2 size = {cast(f32, resized.Width), cast(f32, resized.Height)};
        // m_Camera.Resize(size);
        // m_SceneRenderer.Resize(size);
        return false;
    });
    m_Camera.HandleEvent(event);
    Application::OnEvent(event);
}

void EditorApplication::Resize(Vector2 size)
{
    ZoneScoped;
    m_Camera.Resize(size);
    m_SceneRenderer.Resize(size);
}