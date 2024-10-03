#include "FussionPCH.h"
#include "Application.h"
#include "Fussion/Input/Input.h"
#include "Fussion/OS/Clock.h"
#include "Fussion/Rendering/Renderer.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Scripting/ScriptingEngine.h"
#include "Time.h"

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

namespace Fussion {
    Application* Application::s_Instance = nullptr;

    class SimpleSink final : public LogSink {
        Application* m_application;

    public:
        explicit SimpleSink(Application* app)
            : m_application(app)
        { }

        virtual void Write(LogLevel level, std::string_view message, std::source_location const& loc) override
        {
            m_application->OnLogReceived(level, message, loc);
        }
    };

    Application::~Application()
    {
        LOG_DEBUGF("Application terminating");
    }

    void Application::Run()
    {
        LOG_DEBUG("Initializing application");
        s_Instance = this;
        Log::DefaultLogger()->SetLogLevel(LogLevel::Debug);
        Log::DefaultLogger()->RegisterSink(MakeRef<SimpleSink>(this));

        WindowOptions options {
            .InitialTitle = "Window",
            .InitialWidth = 1366,
            .InitialHeight = 768,
            .Flags = WindowFlag::Centered | WindowFlag::Decorated,
        };
        m_Window.reset(Window::Create(options));
        m_Window->SetEventCallback([this](Event& event) -> bool {
            ZoneScoped;
            Input::OnEvent(event);

            OnEvent(event);
            return false;
        });

        ScriptingEngine::Initialize();
        defer(ScriptingEngine::Shutdown());

        Renderer::Initialize(*m_Window.get());

        OnStart();

        Renderer::Self().CreateDefaultResources();

        Clock clock;
        while (!m_QuitRequested) {
            ZoneScopedN("Main Loop");

            auto const delta = CAST(f32, clock.Reset());
            Time::SetDeltaTime(delta);

            m_Window->Update();
            OnUpdate(delta);

            Input::Flush();
            FrameMark;
        }

        Renderer::Shutdown();
    }

    void Application::Quit()
    {
        LOG_DEBUG("Quit was requested");
        m_QuitRequested = true;
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
