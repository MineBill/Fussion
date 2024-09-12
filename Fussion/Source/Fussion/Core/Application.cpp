#include "FussionPCH.h"
#include "Application.h"
#include "Time.h"
#include "Fussion/Input/Input.h"
#include "Fussion/OS/Clock.h"
#include "Fussion/Rendering/Renderer.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Scripting/ScriptingEngine.h"

#include <tracy/TracyC.h>
#include <tracy/Tracy.hpp>

namespace Fussion {
    Application* Application::s_instance = nullptr;

    class SimpleSink final : public LogSink {
        Application* m_application;

    public:
        explicit SimpleSink(Application* app): m_application(app) {}

        virtual void write(LogLevel level, std::string_view message, std::source_location const& loc) override
        {
            m_application->on_log_received(level, message, loc);
        }
    };

    Application::~Application()
    {
        LOG_DEBUGF("Application terminating");
    }

    void Application::run()
    {
        LOG_DEBUG("Initializing application");
        s_instance = this;
        Log::default_logger()->set_log_level(LogLevel::Debug);
        Log::default_logger()->register_sink(make_ref<SimpleSink>(this));

        WindowOptions options{
            .initial_title = "Window",
            .initial_width = 1366,
            .initial_height = 768,
            .flags = WindowFlag::Centered | WindowFlag::Decorated,
        };
        m_window.reset(Window::create(options));
        m_window->on_event([this](Event& event) -> bool {
            ZoneScoped;
            Input::on_event(event);

            on_event(event);
            return false;
        });

        ScriptingEngine::initialize();
        defer(ScriptingEngine::shutdown());

        Renderer::initialize(*m_window.get());

        on_start();

        Renderer::inst().create_default_resources();

        Clock clock;
        while (!m_quit) {
            ZoneScopedN("Main Loop");

            auto const delta = CAST(f32, clock.reset());
            Time::set_delta_time(delta);

            m_window->update();
            on_update(delta);

            Input::flush();
            FrameMark;
        }

        Renderer::shutdown();
    }

    void Application::quit()
    {
        LOG_DEBUG("Quit was requested");
        m_quit = true;
    }
}

void* operator new(std::size_t size)
{
    auto ptr = malloc(size);
    TracyAllocS(ptr, size, 15);
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    TracyFreeS(ptr, 15);
    free(ptr);
}
