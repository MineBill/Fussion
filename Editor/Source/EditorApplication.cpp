#include "EditorPCH.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "EditorApplication.h"

#include "Layers/ImGuiLayer.h"
#include "Project/Project.h"
#include "Layers/ProjectCreatorLayer.h"
#include "Layers/Editor.h"

#include <Fussion/Rendering/Renderer.h>
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

    auto image = TextureImporter::LoadImageFromMemory({ g_logo_32_data }).Value();
    m_Window->SetIcon(image);

    m_ImGuiLayer = PushLayer<ImGuiLayer>();
    m_ImGuiLayer->Init();

    EditorStyle::GetStyle().Initialize();

    if (m_Args.CreateProject) {
        if (auto project = m_Args.ProjectPath) {
            auto path = CreateProject(std::filesystem::path(*project));
            CreateEditor(path);
        } else {
            PANIC("Must provide path with the create option");
        }
    } else {
        if (auto project = m_Args.ProjectPath) {
            CreateEditor(std::filesystem::path(*project));
        } else {
            PushLayer<ProjectCreatorLayer>();
        }
    }

    Application::OnStart();
}

void EditorApplication::OnUpdate(f32 delta)
{
    ZoneScoped;
    using namespace Fussion;

    m_ImGuiLayer->Begin();

    Application::OnUpdate(delta);

    auto view = Renderer::Begin();
    if (!view) {
        m_ImGuiLayer->End(None());
        return;
    }

    auto encoder = Renderer::Device().CreateCommandEncoder();

    std::array color_attachments{
        GPU::RenderPassColorAttachment{
            .View = *view,
            .LoadOp = GPU::LoadOp::Clear,
            .StoreOp = GPU::StoreOp::Store,
            .ClearColor = Color::Coral,
        }
    };
    GPU::RenderPassSpec rp_spec{
        .Label = "Main RenderPass"sv,
        .ColorAttachments = color_attachments
    };

    for (auto const& layer : m_Layers) {
        layer->OnDraw(encoder);
    }

    auto main_rp = encoder.BeginRendering(rp_spec);

    m_ImGuiLayer->End(main_rp);

    main_rp.End();

    view->Release();
    Renderer::End(encoder.Finish());

    // auto [cmd, image] = Renderer::Begin();
    // if (cmd == nullptr) {
    //     m_ImGuiLayer->End(cmd);
    //     return;
    // }
    //
    // cmd->Begin();
    // auto window_size = Vector2{ CAST(f32, m_Window->GetWidth()), CAST(f32, m_Window->GetHeight()) };
    //
    // for (auto const& layer : m_Layers) {
    //     layer->OnDraw(cmd);
    // }
    //
    // auto main = Renderer::GetInstance()->GetMainRenderPass();
    // cmd->BeginRenderPass(main, Renderer::GetInstance()->GetSwapchain()->GetFrameBuffer(image));
    // cmd->SetViewport({ window_size.X, -window_size.Y });
    // cmd->SetScissor({ 0, 0, window_size.X, window_size.Y });
    //
    // m_ImGuiLayer->End(cmd);
    //
    // cmd->EndRenderPass(main);
    //
    // cmd->End();
    //
    // Renderer::End(cmd);
}

void EditorApplication::OnEvent(Event& event)
{
    Application::OnEvent(event);

    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowResized>([](WindowResized const& e) {
        Renderer::Resize({ e.Width, e.Height });
        return false;
    });
}

void EditorApplication::OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc)
{
    Application::OnLogReceived(level, message, loc);
}

auto EditorApplication::CreateProject(Maybe<fs::path> path) -> fs::path
{
    if (path.IsEmpty() || !exists(*path) || !is_directory(*path)) {
        path = Dialogs::ShowDirectoryPicker();
    }

    return Project::GenerateProject(*path, "Simple Project");
}

void EditorApplication::CreateEditor(Maybe<fs::path> path)
{
    if (path.IsEmpty() || !exists(*path)) {
        path = Dialogs::ShowFilePicker("Fussion Project", { "*.fsnproj" })[0];
    }

    bool loaded = Project::Load(*path);
    VERIFY(loaded, "Project loading must not fail, for now.");

    (void)s_EditorInstance->PushLayer<Editor>();

    auto now = std::chrono::system_clock::now();
    auto log_file = std::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::DefaultLogger()->RegisterSink(FileSink::Create(Project::GetLogsFolder() / log_file));

}

void EditorApplication::CreateEditorFromProjectCreator(fs::path path)
{
    s_EditorInstance->PopLayer();

    if (!exists(path)) {
        path = Dialogs::ShowFilePicker("Fussion Project", { "*.fsnproj" })[0];
    }

    bool loaded = Project::Load(path);
    VERIFY(loaded, "Project loading must not fail, for now.");

    auto editor = s_EditorInstance->PushLayer<Editor>();
    editor->OnStart();

    auto now = std::chrono::system_clock::now();
    auto log_file = std::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::DefaultLogger()->RegisterSink(FileSink::Create(Project::GetLogsFolder() / log_file));
}
