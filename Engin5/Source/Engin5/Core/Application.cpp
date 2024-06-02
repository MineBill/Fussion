#include "e5pch.h"
#include "Application.h"

#include <tracy/Tracy.hpp>

#include "Engin5/Input/Input.h"
#include "Engin5/OS/Clock.h"
#include "Engin5/Renderer/Renderer.h"

namespace Engin5
{
    Application* Application::s_Instance = nullptr;

    void Application::Run()
    {
        s_Instance = this;
        // Setup the engine

        const WindowOptions options {
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

        Log::DefaultLogger()->SetLogLevel(LogLevel::Debug);

        Renderer::Init(*m_Window.get());
        Renderer::GetInstance()->CreateDefaultResources();

        OnStart();

        Clock clock;
        while (!m_Window->ShouldClose()) {
            const auto delta = cast(f32, clock.Reset()) / 1000.0f;
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
}
