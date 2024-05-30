#pragma once
#include "e5pch.h"
#include "Engin5/Core/Core.h"

#define EVENT_BIND_FN(fn) [this](auto &&PH1) { return fn(std::forward<decltype(PH1)>(PH1)); }

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

#define EVENT_TOSTRING(name)          \
    std::string ToString() const override \
    {                                 \
        return #name;                 \
    }

namespace Engin5
{
    enum class EventType {
        OnKeyPressed = 0,
        OnKeyReleased,
        OnKeyDown,
        WindowClosed,
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

    class Event
    {
        friend class EventDispatcher;

    public:
        bool Handled{ false };

        virtual ~Event() = default;

        require_results virtual EventType Type() const = 0;
        require_results virtual std::string ToString() const = 0;
    };

    using EventFnType = std::function<bool(Event&)>;

    class EventDispatcher
    {
        Event* m_Event;

    public:
        template<std::derived_from<Event> T>
        using EventFn = std::function<bool (T&)>;

        explicit EventDispatcher(Event& e) : m_Event(&e)
        {
        }

        template<std::derived_from<Event> T>
        void Dispatch(EventFn<T> fn)
        {
            if (m_Event->Handled || m_Event->Type() != T::StaticType())
                return;

            m_Event->Handled = fn(dynamic_cast<T&>(*m_Event));
        }
    };

} // namespace Fussion