#pragma once
#include "Layer.h"
#include "Types.h"
#include "Engin5/Window.h"
#include "Engin5/Events/Event.h"

namespace Engin5
{
    class Application
    {
    public:

        virtual void OnStart()
        {
            for (const auto& layer : m_Layers) {
                layer->OnStart();
            }
        }

        virtual void OnUpdate(f32)
        {
            for (const auto& layer : m_Layers) {
                layer->OnUpdate();
            }
        }
        virtual void OnEvent(Event&) {}

        Window const* GetWindow() const { return m_Window.get(); }
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
