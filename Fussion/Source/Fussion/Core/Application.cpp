#include "e5pch.h"
#include "Application.h"

#include "Time.h"

#include <tracy/Tracy.hpp>

#include "Fussion/Input/Input.h"
#include "Fussion/OS/Clock.h"
#include "Fussion/RHI/Renderer.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Scripting/ScriptingEngine.h"

namespace Fussion {
Application* Application::s_Instance = nullptr;

class SimpleSink : public LogSink {
    Application* m_Application;

public:
    explicit SimpleSink(Application* app): m_Application(app) {}

    void Write(const LogLevel level, const std::string_view message, std::source_location const& loc) override
    {
        m_Application->OnLogReceived(level, message, loc);
    }
};

void Application::Run()
{
    s_Instance = this;
    Log::DefaultLogger()->SetLogLevel(LogLevel::Debug);
    Log::DefaultLogger()->RegisterSink(MakeRef<SimpleSink>(this));

    const WindowOptions options{
        .InitialTitle = "Window",
        .InitialWidth = 1366,
        .InitialHeight = 768,
        .Flags = WindowFlag::Centered,
    };
    m_Window.reset(Window::Create(options));
    m_Window->OnEvent([this](Event& event) -> bool {
        ZoneScoped;
        OnEvent(event);
        Input::OnEvent(event);
        for (const auto& layer : m_Layers) {
            if (event.Handled) {
                break;
            }
            layer->OnEvent(event);
        }
        return false;
    });

    ScriptingEngine::Initialize();
    defer(ScriptingEngine::Shutdown());

    RHI::Renderer::Init(*m_Window.get());
    RHI::Renderer::GetInstance()->CreateDefaultRenderpasses();

    OnStart();

    RHI::Renderer::GetInstance()->CreateDefaultResources();

    Clock clock;
    while (!m_Quit) {
        const auto delta = CAST(f32, clock.Reset()) / 1000.0f;
        Time::SetDeltaTime(delta);
        m_Window->Update();
        OnUpdate(delta);

        Input::Flush();
        FrameMark;
    }
}

void Application::PushLayer(Layer* layer)
{
    m_Layers.push_back(layer);
}

void Application::Quit()
{
    LOG_DEBUG("Quit was requested");
    m_Quit = true;
}
}
