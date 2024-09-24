#pragma once
#include "Event.h"
#include "Fussion//Core/Types.h"
#include <Fussion/Math/Vector2.h>

namespace Fussion {
    enum class [[API]] MouseButton {
        None,
        Left,
        Right,
        Middle,
        Button4,
        Button5,
        Button6,
        Button7,
        Button8
    };


    class MouseMoved final : public Event {
    public:
        EVENT(MouseMoved)
        MouseMoved(f64 x, f64 y, f64 rel_x, f64 rel_y) : x(x), y(y), rel_x(rel_x), rel_y(rel_y) {}

        f64 x{ 0 }, y{ 0 }, rel_x{ 0 }, rel_y{ 0 };
    };

    class MouseButtonPressed final : public Event {
    public:
        EVENT(MouseButtonPressed)
        explicit MouseButtonPressed(MouseButton button) : button(button) {}

        MouseButton button{};
    };

    class MouseButtonReleased final : public Event {
    public:
        EVENT(MouseButtonReleased)
        explicit MouseButtonReleased(MouseButton b) : button(b) {}

        MouseButton button{};
    };

    class MouseButtonDown final : public Event {
    public:
        EVENT(MouseButtonDown)
        explicit MouseButtonDown(MouseButton button) : button(button) {}

        MouseButton button{};
    };

    class MouseWheelMoved final : public Event {
    public:
        EVENT(MouseWheelMoved)
        explicit MouseWheelMoved(f32 x, f32 y) : x(x), y(y) {}

        Vector2 offset() const
        {
            return { x, y };
        }

        f32 x, y{ 0.0f };
    };
}
