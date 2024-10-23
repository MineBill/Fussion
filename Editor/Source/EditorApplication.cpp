#include "EditorPCH.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Fussion/OS/FileSystem.h"

#include "EditorApplication.h"
#include "Layers/Editor.h"
#include "Layers/ImGuiLayer.h"
#include "Layers/ProjectCreatorLayer.h"
#include "Project/Project.h"

#include <Fussion/Events/ApplicationEvents.h>
#include <Fussion/Input/Input.h>
#include <Fussion/Log/FileSink.h>
#include <Fussion/OS/Args.h>
#include <Fussion/OS/Dialog.h>
#include <Fussion/Rendering/Renderer.h>
#include <Fussion/Util/TextureImporter.h>
#include <chrono>
#include <tracy/Tracy.hpp>

using namespace Fussion;

namespace {
    unsigned char LOGO32_DATA[] = {
#include "logo_32.png.h"
    };
}

EditorApplication* EditorApplication::s_EditorInstance;

Ptr<ProjectCreatorLayer> g_ProjectCreator;
Ptr<Editor> g_Editor;
Ptr<ImGuiLayer> g_Imgui;

EditorApplication::EditorApplication()
{
    m_Args = argparse::parse<EditorCLI>(Args::Argc(), Args::Argv());
}

void EditorApplication::OnStart()
{
    ZoneScoped;

    s_EditorInstance = this;

    Project::Initialize();

    auto image = TextureImporter::LoadImageFromMemory({ LOGO32_DATA }).Unwrap();
    m_Window->SetIcon(image);

    g_Imgui = MakePtr<ImGuiLayer>();
    g_Imgui->Initialize();

    EditorStyle::Style().Initialize();

    if (m_Args.CreateProject) {
        if (auto project = m_Args.ProjectPath) {
            auto path = CreateProject(std::filesystem::path(*project), "EmptyProject");
            CreateEditor(path);
        } else {
            PANIC("Must provide path with the create option");
        }
    } else {
        if (auto project = m_Args.ProjectPath) {
            CreateEditor(std::filesystem::path(*project));
        } else {
            g_ProjectCreator = MakePtr<ProjectCreatorLayer>();
            g_ProjectCreator->OnStart();
        }
    }

    Application::OnStart();
}

void EditorApplication::OnUpdate(f32 delta)
{
    ZoneScoped;
    using namespace Fussion;

    g_Imgui->Begin();

    if (g_ProjectCreator)
        g_ProjectCreator->OnUpdate(delta);
    if (g_Editor)
        g_Editor->OnUpdate(delta);

    auto view = Renderer::BeginRendering();
    if (!view) {
        g_Imgui->End(None());
        return;
    }

    auto encoder = Renderer::Device().CreateCommandEncoder();

    std::array colorAttachments {
        GPU::RenderPassColorAttachment {
            .View = *view,
            .LoadOp = GPU::LoadOp::Clear,
            .StoreOp = GPU::StoreOp::Store,
            .ClearColor = Color::Coral,
        }
    };
    GPU::RenderPassSpec rp_spec {
        .Label = "Main RenderPass"sv,
        .ColorAttachments = colorAttachments
    };

    if (g_ProjectCreator)
        g_ProjectCreator->OnDraw(encoder);
    if (g_Editor)
        g_Editor->OnDraw(encoder);

    auto main_rp = encoder.BeginRendering(rp_spec);

    g_Imgui->End(main_rp);

    main_rp.End();
    main_rp.Release();

    auto cmd = encoder.Finish();
    Renderer::EndRendering(cmd);
    encoder.Release();
    view->Release();
}

void EditorApplication::OnEvent(Event& event)
{
    if (g_ProjectCreator)
        g_ProjectCreator->OnEvent(event);
    if (g_Editor)
        g_Editor->OnEvent(event);

    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowResized>([](WindowResized const& e) {
        Renderer::Resize({ e.Width, e.Height });
        return false;
    });
}

void EditorApplication::OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc)
{
    if (g_ProjectCreator)
        g_ProjectCreator->OnLogReceived(level, message, loc);
    if (g_Editor)
        g_Editor->OnLogReceived(level, message, loc);
}

auto EditorApplication::CreateProject(Maybe<fs::path> path, std::string_view name) -> fs::path
{
    if (path.IsEmpty() || !is_directory(*path)) {
        path = Dialogs::ShowDirectoryPicker();
    }

    return Project::GenerateProject(*path, name);
}

void EditorApplication::CreateEditor(Maybe<fs::path> path)
{
    if (path.IsEmpty() || !exists(*path)) {
        path = Dialogs::ShowFilePicker("Fussion Project", { "*.fsnproj" })[0];
    }

    bool loaded = Project::Load(*path);
    VERIFY(loaded, "Project loading must not fail, for now.");

    g_Editor = MakePtr<Editor>();

    auto now = std::chrono::system_clock::now();
    auto log_file = fmt::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::DefaultLogger()->RegisterSink(FileSink::Create(Project::LogsFolderPath() / log_file));
}

void EditorApplication::CreateEditorFromProjectCreator(fs::path path)
{
    (void)g_ProjectCreator.release();

    if (!exists(path)) {
        path = Dialogs::ShowFilePicker("Fussion Project", { "*.fsnproj" })[0];
    }

    bool loaded = Project::Load(path);
    VERIFY(loaded, "Project loading must not fail, for now.");

    g_Editor = MakePtr<Editor>();
    g_Editor->OnStart();

    auto now = std::chrono::system_clock::now();
    auto log_file = fmt::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::DefaultLogger()->RegisterSink(FileSink::Create(Project::LogsFolderPath() / log_file));
}
