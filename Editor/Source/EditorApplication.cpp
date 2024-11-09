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
#include <Fussion/Util/TextureLoader.h>
#include <chrono>
#include <tracy/Tracy.hpp>

using namespace Fussion;

#ifdef IS_XMAKE
namespace {
    unsigned char LOGO32_DATA[] = {
#    include "logo_32.png.h"
    };
}
#else
#    include "battery/embed.hpp"
#endif

EditorApplication* EditorApplication::s_editor_instance;

Ptr<ProjectCreatorLayer> g_ProjectCreator;
Ptr<Editor> g_Editor;
Ptr<ImGuiLayer> g_Imgui;

EditorApplication::EditorApplication()
{
    m_args = argparse::parse<EditorCLI>(Args::argc(), Args::argv());
}

void EditorApplication::on_start()
{
    ZoneScoped;

    s_editor_instance = this;

    Project::initialize();

#ifndef IS_XMAKE
    auto LOGO32_DATA = b::embed<"Assets/Icons/logo_32.png">().vec();
#endif
    auto image = TextureLoader::load_image_from_memory(LOGO32_DATA).unwrap();
    m_window->set_icon(image);

    g_Imgui = make_ptr<ImGuiLayer>();
    g_Imgui->initialize();

    EditorStyle::style().initialize();

    if (m_args.create_project) {
        if (auto project = m_args.project_path) {
            auto path = create_project(std::filesystem::path(*project), "EmptyProject");
            create_editor(path);
        } else {
            PANIC("Must provide path with the create option");
        }
    } else {
        if (auto project = m_args.project_path) {
            create_editor(std::filesystem::path(*project));
        } else {
            g_ProjectCreator = make_ptr<ProjectCreatorLayer>();
            g_ProjectCreator->on_start();
        }
    }

    Application::on_start();
}

void EditorApplication::on_update(f32 delta)
{
    ZoneScoped;
    using namespace Fussion;

    g_Imgui->begin();

    if (g_ProjectCreator)
        g_ProjectCreator->on_update(delta);
    if (g_Editor)
        g_Editor->on_update(delta);

    auto view = Renderer::begin_rendering();
    if (!view) {
        g_Imgui->end(None());
        return;
    }

    auto encoder = Renderer::device().create_command_encoder();

    std::array colorAttachments {
        GPU::RenderPassColorAttachment {
            .view = *view,
            .load_op = GPU::LoadOp::Clear,
            .store_op = GPU::StoreOp::Store,
            .clear_color = Color::Coral,
        }
    };
    GPU::RenderPassSpec rp_spec {
        .label = "Main RenderPass"sv,
        .color_attachments = colorAttachments
    };

    if (g_ProjectCreator)
        g_ProjectCreator->on_draw(encoder);
    if (g_Editor)
        g_Editor->on_draw(encoder);

    auto main_rp = encoder.begin_rendering(rp_spec);

    g_Imgui->end(main_rp);

    main_rp.end();
    main_rp.release();

    auto cmd = encoder.finish();
    Renderer::end_rendering(cmd);
    encoder.release();
    view->release();
}

void EditorApplication::on_event(Event& event)
{
    if (g_ProjectCreator)
        g_ProjectCreator->on_event(event);
    if (g_Editor)
        g_Editor->on_event(event);

    EventDispatcher dispatcher(event);
    dispatcher.dispatch<WindowResized>([](WindowResized const& e) {
        Renderer::resize({ e.width, e.height });
        return false;
    });
}

void EditorApplication::on_log_received(LogLevel level, std::string_view message, std::source_location const& loc)
{
    if (g_ProjectCreator)
        g_ProjectCreator->on_log_received(level, message, loc);
    if (g_Editor)
        g_Editor->on_log_received(level, message, loc);
}

auto EditorApplication::create_project(Maybe<fs::path> path, std::string_view name) -> fs::path
{
    if (path.is_empty() || !is_directory(*path)) {
        path = Dialogs::show_directory_picker();
    }

    return Project::generate_project(*path, name);
}

void EditorApplication::create_editor(Maybe<fs::path> path)
{
    if (path.is_empty() || !exists(*path)) {
        path = Dialogs::show_file_picker("Fussion Project", { "*.fsnproj" })[0];
    }

    bool loaded = Project::load(*path);
    VERIFY(loaded, "Project loading must not fail, for now.");

    g_Editor = make_ptr<Editor>();

    auto now = std::chrono::system_clock::now();
    auto log_file = fmt::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::default_logger()->register_sink(FileSink::Create(Project::logs_folder_path() / log_file));
}

void EditorApplication::create_editor_from_project_creator(fs::path path)
{
    (void)g_ProjectCreator.release();

    if (!exists(path)) {
        path = Dialogs::show_file_picker("Fussion Project", { "*.fsnproj" })[0];
    }

    bool loaded = Project::load(path);
    VERIFY(loaded, "Project loading must not fail, for now.");

    g_Editor = make_ptr<Editor>();
    g_Editor->on_start();

    auto now = std::chrono::system_clock::now();
    auto log_file = fmt::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::default_logger()->register_sink(FileSink::Create(Project::logs_folder_path() / log_file));
}
