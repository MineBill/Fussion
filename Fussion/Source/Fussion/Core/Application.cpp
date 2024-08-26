#include "FussionPCH.h"
#include "Application.h"

#include "Time.h"

#include <tracy/Tracy.hpp>

#include "Fussion/Input/Input.h"
#include "Fussion/OS/Clock.h"
#include "Fussion/RHI/Renderer.h"
#include "Fussion/Scene/Components/BaseComponents.h"
#include "Scripting/ScriptingEngine.h"

#include <tracy/TracyC.h>

namespace Fussion {
Application* Application::s_Instance = nullptr;

class SimpleSink final : public LogSink {
    Application* m_Application;

public:
    explicit SimpleSink(Application* app): m_Application(app) {}

    virtual void Write(LogLevel level, std::string_view message, std::source_location const& loc) override
    {
        m_Application->OnLogReceived(level, message, loc);
    }
};

Application::~Application()
{
    LOG_DEBUGF("Application terminating");
}

void Application::Run()
{
    s_Instance = this;
    Log::DefaultLogger()->SetLogLevel(LogLevel::Debug);
    Log::DefaultLogger()->RegisterSink(MakeRef<SimpleSink>(this));

    WindowOptions options{
        .InitialTitle = "Window",
        .InitialWidth = 1366,
        .InitialHeight = 768,
        .Flags = WindowFlag::Centered | WindowFlag::Decorated,
    };
    m_Window.reset(Window::Create(options));
    m_Window->OnEvent([this](Event& event) -> bool {
        ZoneScoped;
        Input::OnEvent(event);

        OnEvent(event);
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
        auto const delta = CAST(f32, clock.Reset()) / 1000.0f;
        Time::SetDeltaTime(delta);
        m_Window->Update();
        OnUpdate(delta);

        Input::Flush();
        FrameMark;
    }

    RHI::Renderer::Shutdown();
}

Layer* Application::PushLayer(Ptr<Layer> layer)
{
    auto ptr = layer.get();
    m_Layers.push_back(std::move(layer));
    return ptr;
}

void Application::PopLayer()
{
    m_Layers.pop_back();
}

void Application::Quit()
{
    LOG_DEBUG("Quit was requested");
    m_Quit = true;
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
