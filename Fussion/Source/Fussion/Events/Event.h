#pragma once
#include <string>
#include <functional>

#define EVENT(name)                 \
    static EventType StaticType()  \
    {                               \
        return EventType::name;     \
    }                               \
                                    \
    EventType Type() const override \
    {                               \
        return StaticType();       \
    }

namespace Fussion {
enum class EventType {
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
    bool Handled{ false };

    virtual ~Event() = default;

    [[nodiscard]]
    virtual EventType Type() const = 0;
};

using EventFnType = std::function<bool(Event&)>;

class EventDispatcher {
    Event* m_Event;

public:
    template<std::derived_from<Event> T>
    using EventFn = std::function<bool (T&)>;

    explicit EventDispatcher(Event& e) : m_Event(&e) {}

    template<std::derived_from<Event> T>
    void Dispatch(EventFn<T> fn)
    {
        if (m_Event->Handled || m_Event->Type() != T::StaticType())
            return;

        m_Event->Handled = fn(dynamic_cast<T&>(*m_Event));
    }
};

} // namespace Fussion
