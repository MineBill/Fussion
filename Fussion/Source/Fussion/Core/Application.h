#pragma once
#include "Layer.h"
#include "Types.h"
#include "Fussion/Window.h"
#include "Fussion/Events/Event.h"

namespace Fussion {
class Application {
public:
    virtual ~Application();

    virtual void OnStart()
    {
        for (auto const& layer : m_Layers) {
            layer->OnStart();
        }
    }

    virtual void OnUpdate(f32 delta)
    {
        for (auto const& layer : m_Layers) {
            layer->OnUpdate(delta);
        }
    }

    virtual void OnEvent(Event& event)
    {
        for (auto const& layer : m_Layers) {
            if (event.Handled) {
                break;
            }
            layer->OnEvent(event);
        }
    }

    virtual void OnLogReceived(
        LogLevel level,
        std::string_view message,
        std::source_location const& loc)
    {
        for (auto const& layer : m_Layers) {
            layer->OnLogReceived(level, message, loc);
        }
    }

    Window& GetWindow() const { return *m_Window.get(); }
    static Application* Instance() { return s_Instance; }

    void Run();

    Layer* PushLayer(Ptr<Layer> layer);

    template<std::derived_from<Layer> T>
    T* PushLayer()
    {
        auto layer = MakePtr<T>();
        return dynamic_cast<T*>(PushLayer(std::move(layer)));
    }

    void Quit();

protected:
    std::vector<Ptr<Layer>> m_Layers{};
    Ptr<Window> m_Window{};
    bool m_Quit{ false };

private:
    static Application* s_Instance;
};
}

namespace Fsn = Fussion;
