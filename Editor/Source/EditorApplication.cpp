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
#include "Engin5/Scene/Entity.h"
#include "Project/Project.h"

EditorApplication* EditorApplication::s_EditorInstance;

void EditorApplication::OnStart()
{
    ZoneScoped;
    using namespace Engin5;

    s_EditorInstance = this;
    Log::DefaultLogger()->RegisterSink(FileSink::Create("Pepegas.log"));
    LOG_DEBUGF("Current path is: {}", std::filesystem::current_path().string());

    s_EditorInstance = this;
    m_Editor = MakePtr<Editor>();

    m_ImGuiLayer = MakePtr<ImGuiLayer>();
    m_ImGuiLayer->Init();

    m_Editor->OnStart();

    PushLayer(m_ImGuiLayer.get());

    Application::OnStart();
}

void EditorApplication::OnUpdate(const f32 delta)
{
    ZoneScoped;
    using namespace Engin5;

    m_ImGuiLayer->Begin();

    // Update the layers.
    Application::OnUpdate(delta);
    m_Editor->OnUpdate(delta);

    auto [cmd, image] = Renderer::Begin();
    if (cmd == nullptr) {
        m_ImGuiLayer->End(cmd);
        return;
    }

    cmd->Begin();
    auto window_size = Vector2{cast(f32, m_Window->GetWidth()), cast(f32, m_Window->GetHeight())};

    m_Editor->OnDraw(cmd);

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
    Application::OnEvent(event);
}

void EditorApplication::OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc)
{
    if (m_Editor)
        m_Editor->OnLogReceived(level, message, loc);
}