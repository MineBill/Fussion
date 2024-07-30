#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "EditorApplication.h"
#include "Layers/ImGuiLayer.h"

#include "Fussion/RHI/Renderer.h"
#include "Fussion/Input/Input.h"
#include "imgui.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

#include "Fussion/Events/ApplicationEvents.h"
#include "Fussion/Log/FileSink.h"
#include "Fussion/OS/Args.h"
#include "Fussion/OS/Dialog.h"
#include "Fussion/OS/FileSystem.h"
#include "Fussion/Scene/Entity.h"
#include "Project/Project.h"
#include <Fussion/Core/Clap.h>
#include <chrono>

EditorApplication* EditorApplication::s_EditorInstance;

void EditorApplication::OnStart()
{
    ZoneScoped;
    using namespace Fussion;

    s_EditorInstance = this;

    Clap clap(Args::AsSingleLine());
    clap.Option<std::string>("Project");

    clap.Parse();

    bool loaded;
    if (auto project = clap.Get<std::string>("Project")) {
        loaded = Project::Load(*project);
    } else {
        auto path = Dialogs::ShowFilePicker("Project File", { "*.fsnproj" });
        loaded = Project::Load(path);
    }
    VERIFY(loaded, "Project loading must not fail, for now.");

    auto now = std::chrono::system_clock::now();
    auto log_file = std::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::DefaultLogger()->RegisterSink(FileSink::Create(Project::ActiveProject()->GetLogsFolder() / log_file));

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
    using namespace Fussion;

    m_ImGuiLayer->Begin();

    // Update the layers.
    Application::OnUpdate(delta);
    m_Editor->OnUpdate(delta);

    auto [cmd, image] = RHI::Renderer::Begin();
    if (cmd == nullptr) {
        m_ImGuiLayer->End(cmd);
        return;
    }

    cmd->Begin();
    auto window_size = Vector2{ CAST(f32, m_Window->GetWidth()), CAST(f32, m_Window->GetHeight()) };

    m_Editor->OnDraw(cmd);

    const auto main = RHI::Renderer::GetInstance()->GetMainRenderPass();
    cmd->BeginRenderPass(main, RHI::Renderer::GetInstance()->GetSwapchain()->GetFrameBuffer(image));
    cmd->SetViewport({ window_size.X, -window_size.Y });
    cmd->SetScissor({ 0, 0, window_size.X, window_size.Y });

    m_ImGuiLayer->End(cmd);

    cmd->EndRenderPass(main);

    cmd->End();

    RHI::Renderer::End(cmd);
}

void EditorApplication::OnEvent(Fussion::Event& event)
{
    using namespace Fussion;
    m_Editor->OnEvent(event);
}

void EditorApplication::OnLogReceived(Fsn::LogLevel level, std::string_view message, std::source_location const& loc)
{
    if (m_Editor)
        m_Editor->OnLogReceived(level, message, loc);
}
