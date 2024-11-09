#pragma once
#include "Fussion/Events/Event.h"
#include "Fussion/Window.h"
#include "Types.h"

namespace Fussion {
    class Application {
    public:
        virtual ~Application();

        virtual void on_start() { }

        virtual void on_update([[maybe_unused]] f32 delta) { }

        virtual void on_event([[maybe_unused]] Event& event) { }

        virtual void on_log_received(
            [[maybe_unused]] LogLevel level,
            [[maybe_unused]] std::string_view message,
            [[maybe_unused]] std::source_location const& loc) { }

        Window& window() const { return *m_window.get(); }
        static Application* self() { return s_instance; }

        void run();

        void quit();

    protected:
        Ptr<Window> m_window {};
        bool m_quit_requested { false };
        Ref<LogSink> m_Sink {};

    private:
        static Application* s_instance;
    };
}

namespace Fsn = Fussion;
