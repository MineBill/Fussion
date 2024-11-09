#pragma once
#include "Event.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"

namespace Fussion {
    /**
     * Sent when the user requests the application to close.
     */
    class WindowCloseRequest final : public Event {
    public:
        EVENT(WindowClose)
        WindowCloseRequest() = default;
    };

    class WindowResized final : public Event {
    public:
        EVENT(WindowResized)

        explicit WindowResized(int w, int h)
            : width(w)
            , height(h)
        { }

        s32 width { 0 };
        s32 height { 0 };
    };

    class WindowMoved final : public Event {
    public:
        EVENT(WindowMoved)
        explicit WindowMoved(unsigned new_x, unsigned new_y)
            : x(new_x)
            , y(new_y)
        { }

        u32 x { 0 }, y { 0 };
    };

    class WindowMinimized final : public Event {
    public:
        EVENT(WindowMinimized)
        WindowMinimized() = default;
    };

    class WindowMaximized final : public Event {
    public:
        EVENT(WindowMaximized)
        WindowMaximized() = default;
    };

    class WindowGainedFocus final : public Event {
    public:
        EVENT(WindowGainedFocus)
        WindowGainedFocus() = default;
    };

    class WindowLostFocus final : public Event {
    public:
        EVENT(WindowLostFocus)
        WindowLostFocus() = default;
    };
}
