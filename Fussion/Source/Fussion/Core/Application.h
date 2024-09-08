#pragma once
#include "Layer.h"
#include "Types.h"
#include "Fussion/Window.h"
#include "Fussion/Events/Event.h"

namespace Fussion {
    class Application {
    public:
        virtual ~Application();

        virtual void on_start()
        {
            for (auto const& layer : m_layers) {
                layer->on_start();
            }
        }

        virtual void on_update(f32 delta)
        {
            for (auto const& layer : m_layers) {
                layer->on_update(delta);
            }
        }

        virtual void on_event(Event& event)
        {
            for (auto const& layer : m_layers) {
                if (event.handled) {
                    break;
                }
                layer->on_event(event);
            }
        }

        virtual void on_log_received(
            LogLevel level,
            std::string_view message,
            std::source_location const& loc)
        {
            for (auto const& layer : m_layers) {
                layer->on_log_received(level, message, loc);
            }
        }

        Window& window() const { return *m_window.get(); }
        static Application* inst() { return s_instance; }

        void run();

        Layer* push_layer(Ptr<Layer> layer);

        template<std::derived_from<Layer> T>
        T* push_layer()
        {
            auto layer = make_ptr<T>();
            return dynamic_cast<T*>(push_layer(std::move(layer)));
        }

        void pop_layer();

        void quit();

    protected:
        std::vector<Ptr<Layer>> m_layers{};
        Ptr<Window> m_window{};
        bool m_quit{ false };

    private:
        static Application* s_instance;
    };
}

namespace Fsn = Fussion;
