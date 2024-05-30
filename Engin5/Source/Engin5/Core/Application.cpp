#include "e5pch.h"
#include "Application.h"
#include "Engin5/Input/Input.h"
#include "Engin5/Renderer/Renderer.h"

namespace Engin5
{
    Application* Application::s_Instance = nullptr;

    void Application::Run()
    {
        s_Instance = this;
        // Setup the engine

        const WindowOptions options {};
        m_Window.reset(Window::Create(options));
        m_Window->OnEvent([this](Event& event) -> bool {
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

        while (!m_Window->ShouldClose()) {
            m_Window->Update();
            OnUpdate(0.0);

            Input::Flush();
        }
    }

    void Application::PushLayer(Layer* layer)
    {
        m_Layers.push_back(layer);
    }
}
