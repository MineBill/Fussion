#pragma once
#include "Types.h"
#include "Fussion/Window.h"
#include "Fussion/Events/Event.h"

namespace Fussion {
    class Application {
    public:
        virtual ~Application();

        virtual void on_start() {}

        virtual void on_update(f32 delta) {}

        virtual void on_event(Event& event) {}

        virtual void on_log_received(
            LogLevel level,
            std::string_view message,
            std::source_location const& loc) {}

        Window& window() const { return *m_window.get(); }
        static Application* inst() { return s_instance; }

        void run();

        void quit();

    protected:
        Ptr<Window> m_window{};
        bool m_quit{ false };

    private:
        static Application* s_instance;
    };
}

namespace Fsn = Fussion;
