#pragma once
#include <functional>

#define EVENT(name)                 \
    static EventType static_type()  \
    {                               \
        return EventType::name;     \
    }                               \
                                    \
    EventType type() const override \
    {                               \
        return static_type();       \
    }

namespace Fussion {
    enum class EventType : u8 {
        OnKeyPressed = 0,
        OnKeyReleased,
        OnKeyDown,
        WindowClose,
        WindowResized,
        WindowMoved,
        WindowMinimized,
        WindowMaximized,
        WindowGainedFocus,
        WindowLostFocus,
        MouseMoved,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseButtonDown,
        MouseWheelMoved,
    };

    class Event {
        friend class EventDispatcher;

    public:
        bool handled{ false };

        virtual ~Event() = default;

        [[nodiscard]]
        virtual EventType type() const = 0;
    };

    using EventFnType = std::function<bool(Event&)>;

    class EventDispatcher {
        Event* m_event;

    public:
        template<std::derived_from<Event> T>
        using EventFn = std::function<bool (T&)>;

        explicit EventDispatcher(Event& e) : m_event(&e) {}

        template<std::derived_from<Event> T>
        void dispatch(EventFn<T> fn)
        {
            if (m_event->handled || m_event->type() != T::static_type())
                return;

            m_event->handled = fn(dynamic_cast<T&>(*m_event));
        }
    };
} // namespace Fussion
