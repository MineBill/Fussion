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

EditorApplication* EditorApplication::s_editor_instance;
using namespace Fussion;

EditorApplication::EditorApplication()
{
    m_args = argparse::parse<EditorCLI>(Args::argc(), Args::argv());
}

void EditorApplication::on_start()
{
    ZoneScoped;

    s_editor_instance = this;

    Project::initialize();

    auto image = TextureImporter::load_image_from_memory({ g_logo_32_data }).value();
    m_window->set_icon(image);

    m_im_gui_layer = push_layer<ImGuiLayer>();
    m_im_gui_layer->Init();

    EditorStyle::get_style().initialize();

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
            push_layer<ProjectCreatorLayer>();
        }
    }

    Application::on_start();
}

void EditorApplication::on_update(f32 delta)
{
    ZoneScoped;
    using namespace Fussion;

    m_im_gui_layer->Begin();

    Application::on_update(delta);

    auto view = Renderer::begin_rendering();
    if (!view) {
        m_im_gui_layer->End(None());
        return;
    }

    auto encoder = Renderer::device().create_command_encoder();

    std::array color_attachments{
        GPU::RenderPassColorAttachment{
            .view = *view,
            .load_op = GPU::LoadOp::Clear,
            .store_op = GPU::StoreOp::Store,
            .clear_color = Color::Coral,
        }
    };
    GPU::RenderPassSpec rp_spec{
        .label = "Main RenderPass"sv,
        .color_attachments = color_attachments
    };

    for (auto const& layer : m_layers) {
        layer->on_draw(encoder);
    }

    auto main_rp = encoder.begin_rendering(rp_spec);

    m_im_gui_layer->End(main_rp);

    main_rp.end();
    main_rp.release();

    view->release();
    Renderer::end_rendering(encoder.finish());
    encoder.release();

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

void EditorApplication::on_event(Event& event)
{
    Application::on_event(event);

    EventDispatcher dispatcher(event);
    dispatcher.dispatch<WindowResized>([](WindowResized const& e) {
        Renderer::resize({ e.Width, e.Height });
        return false;
    });
}

void EditorApplication::on_log_received(LogLevel level, std::string_view message, std::source_location const& loc)
{
    Application::on_log_received(level, message, loc);
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

    (void)s_editor_instance->push_layer<Editor>();

    auto now = std::chrono::system_clock::now();
    auto log_file = std::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::default_logger()->register_sink(FileSink::Create(Project::logs_folder() / log_file));

}

void EditorApplication::create_editor_from_project_creator(fs::path path)
{
    s_editor_instance->pop_layer();

    if (!exists(path)) {
        path = Dialogs::show_file_picker("Fussion Project", { "*.fsnproj" })[0];
    }

    bool loaded = Project::load(path);
    VERIFY(loaded, "Project loading must not fail, for now.");

    auto editor = s_editor_instance->push_layer<Editor>();
    editor->on_start();

    auto now = std::chrono::system_clock::now();
    auto log_file = std::format("{:%y-%m-%d_%H-%M}.log", now);

    Log::default_logger()->register_sink(FileSink::Create(Project::logs_folder() / log_file));
}
