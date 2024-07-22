#pragma once
#include "Event.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"

#include <format>

namespace Fussion {

/**
 * Sent when the user requests the application to close.
 */
class WindowCloseRequest final : public Event {
public:
    EVENT(WindowClose)
    WindowCloseRequest() = default;

    [[nodiscard]]
    std::string ToString() const override
    {
        return "WindowCloseRequest";
    }
};

class WindowResized final : public Event {
public:
    EVENT(WindowResized)

    explicit WindowResized(int w, int h) : Width(w), Height(h) {}

    require_results std::string ToString() const override
    {
        return std::format("WindowResized(width: {}, height: {})", Width, Height);
    }

    s32 Width{ 0 };
    s32 Height{ 0 };
};

class WindowMoved : public Event {
public:
    EVENT(WindowMoved)
    explicit WindowMoved(unsigned new_x, unsigned new_y) : X(new_x), Y(new_y) {}

    require_results std::string ToString() const override
    {
        return std::format("WindowMoved({}, {})", X, Y);
    }

    u32 X{ 0 }, Y{ 0 };
};

class WindowMinimized : public Event {
public:
    EVENT(WindowMinimized)
    EVENT_TOSTRING(WindowMinimized)
    WindowMinimized() = default;
};

class WindowMaximized : public Event {
public:
    EVENT(WindowMaximized)
    EVENT_TOSTRING(WindowMaximized)
    WindowMaximized() = default;
};

class WindowGainedFocus : public Event {
public:
    EVENT(WindowGainedFocus)
    EVENT_TOSTRING(WindowGainedFocus)
    WindowGainedFocus() = default;
};

class WindowLostFocus : public Event {
public:
    EVENT(WindowLostFocus)
    EVENT_TOSTRING(WindowLostFocus)
    WindowLostFocus() = default;
};
}
