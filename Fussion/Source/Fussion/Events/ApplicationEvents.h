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

        explicit WindowResized(int w, int h) : Width(w), Height(h) {}

        s32 Width{ 0 };
        s32 Height{ 0 };
    };

    class WindowMoved final : public Event {
    public:
        EVENT(WindowMoved)
        explicit WindowMoved(unsigned new_x, unsigned new_y) : X(new_x), Y(new_y) {}

        u32 X{ 0 }, Y{ 0 };
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
