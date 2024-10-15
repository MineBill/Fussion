#pragma once
#include "Fussion/Events/Event.h"
#include "Fussion/Window.h"
#include "Types.h"

namespace Fussion {
    class Application {
    public:
        virtual ~Application();

        virtual void OnStart() { }

        virtual void OnUpdate([[maybe_unused]] f32 delta) { }

        virtual void OnEvent([[maybe_unused]] Event& event) { }

        virtual void OnLogReceived(
            [[maybe_unused]] LogLevel level,
            [[maybe_unused]] std::string_view message,
            [[maybe_unused]] std::source_location const& loc) { }

        Window& GetWindow() const { return *m_Window.get(); }
        static Application* Self() { return s_Instance; }

        void Run();

        void Quit();

    protected:
        Ptr<Window> m_Window {};
        bool m_QuitRequested { false };
        Ref<LogSink> m_Sink {};

    private:
        static Application* s_Instance;
    };
}

namespace Fsn = Fussion;
