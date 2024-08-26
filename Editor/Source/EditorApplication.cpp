#include "EditorPCH.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "EditorApplication.h"
#include "Layers/ImGuiLayer.h"
#include "Project/Project.h"
#include "Layers/ProjectCreatorLayer.h"
#include "Layers/Editor.h"

#include <Fussion/RHI/Renderer.h>
#include <Fussion/Input/Input.h>
#include <Fussion/Events/ApplicationEvents.h>
#include <Fussion/Log/FileSink.h>
#include <Fussion/OS/Args.h>
#include <Fussion/OS/Dialog.h>
#include <Fussion/Util/TextureImporter.h>

#include <tracy/Tracy.hpp>
#include <chrono>

static unsigned char g_logo_32_data[] = {
#include "logo_32.png.h"
};

// static unsigned char g_logo_64_data[] = {
// #include "logo_64.png.h"
// };

EditorApplication* EditorApplication::s_EditorInstance;
using namespace Fussion;

EditorApplication::EditorApplication()
{
    m_Args = argparse::parse<EditorCLI>(Args::Argc(), Args::Argv());
}

void EditorApplication::OnStart()
{
    ZoneScoped;

    s_EditorInstance = this;

    Project::Initialize();

    auto image = TextureImporter::LoadImageFromMemory({ g_logo_32_data });
    m_Window->SetIcon(image);

    m_ImGuiLayer = PushLayer<ImGuiLayer>();
    m_ImGuiLayer->Init();

    EditorStyle::GetStyle().Initialize();

    if (auto project = m_Args.ProjectPath) {
        CreateEditor(std::filesystem::path(*project));
    } else {
        PushLayer<ProjectCreatorLayer>();
    }

    m_ImGuiLayer->LoadFonts();

    Application::OnStart();
}

void EditorApplication::OnUpdate(f32 delta)
{
    ZoneScoped;
    using namespace Fussion;

    m_ImGuiLayer->Begin();

    Application::OnUpdate(delta);

    auto [cmd, image] = RHI::Renderer::Begin();
    if (cmd == nullptr) {
        m_ImGuiLayer->End(cmd);
        return;
    }

    cmd->Begin();
    auto window_size = Vector2{ CAST(f32, m_Window->GetWidth()), CAST(f32, m_Window->GetHeight()) };

    for (auto const& layer : m_Layers) {
        layer->OnDraw(cmd);
    }

    auto main = RHI::Renderer::GetInstance()->GetMainRenderPass();
    cmd->BeginRenderPass(main, RHI::Renderer::GetInstance()->GetSwapchain()->GetFrameBuffer(image));
    cmd->SetViewport({ window_size.X, -window_size.Y });
    cmd->SetScissor({ 0, 0, window_size.X, window_size.Y });

    m_ImGuiLayer->End(cmd);

    cmd->EndRenderPass(main);

    cmd->End();

    RHI::Renderer::End(cmd);
}

void EditorApplication::OnEvent(Event& event)
{
    Application::OnEvent(event);
}

void EditorApplication::OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc)
{
    Application::OnLogReceived(level, message, loc);
}

void EditorApplication::CreateEditor(Maybe<std::filesystem::path> path)
{
    if (path.IsEmpty() || !exists(*path)) {
        path = Dialogs::ShowFilePicker("Fussion Project", { "*.fsnproj" });
    }

    bool loaded = Project::Load(*path);
    VERIFY(loaded, "Project loading must not fail, for now.");

    (void)s_EditorInstance->PushLayer<Editor>();

    auto now = std::chrono::system_clock::now();
    auto log_file = std::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::DefaultLogger()->RegisterSink(FileSink::Create(Project::GetLogsFolder() / log_file));

}
