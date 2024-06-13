#pragma once
#include "Layer.h"
#include "Types.h"
#include "Fussion/Window.h"
#include "Fussion/Events/Event.h"

namespace Fussion
{
    class Application
    {
    public:
        virtual ~Application() = default;

        virtual void OnStart()
        {
            for (const auto& layer : m_Layers) {
                layer->OnStart();
            }
        }

        virtual void OnUpdate(const f32 delta)
        {
            for (const auto& layer : m_Layers) {
                layer->OnUpdate(delta);
            }
        }

        virtual void OnEvent(Event&) {}

        virtual void OnLogReceived(LogLevel level, std::string_view message, std::source_location const& loc) {}

        Window& GetWindow() const { return *m_Window.get(); }
        static Application* Instance() { return s_Instance; }

        void Run();

        void PushLayer(Layer* layer);

    protected:
        std::vector<Layer*> m_Layers{};
        Ptr<Window> m_Window{};

    private:
        static Application* s_Instance;
    };
}

namespace Fsn = Fussion;