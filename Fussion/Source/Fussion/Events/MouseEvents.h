﻿#pragma once
#include "Event.h"
#include "Fussion//Core/Types.h"

#include <format>

#include "Fussion/Core/Core.h"

namespace Fussion
{

    class MouseMoved final : public Event
    {
    public:
        EVENT(MouseMoved)
        MouseMoved(f64 x, f64 y, f64 rel_x, f64 rel_y) : X(x), Y(y), RelX(rel_x), RelY(rel_y)
        {
        }

        require_results std::string ToString() const override
        {
            return std::format("MouseMoved(x: {}, y: {})", X, Y);
        }

        f64 X{0}, Y{0}, RelX{0}, RelY{0};
    };

    enum class MouseButton {
        None,
        Left,
        Right,
        Middle,
        Button4,
        Button5,
        Button6,
        Button7,
        Button8,
    };

    class MouseButtonPressed final : public Event
    {
    public:
        EVENT(MouseButtonPressed)
        explicit MouseButtonPressed(MouseButton button) : Button(button)
        {
        }

        require_results std::string ToString() const override
        {
            return std::format("MouseButtonPressed({})", static_cast<s32>(Button));
        }

        MouseButton Button{};
    };

    class MouseButtonReleased final : public Event
    {
    public:
        EVENT(MouseButtonReleased)
        explicit MouseButtonReleased(MouseButton b) : Button(b)
        {
        }

        require_results std::string ToString() const override
        {
            return std::format("MouseButtonReleased({})", static_cast<s32>(Button));
        }

        MouseButton Button{};
    };

    class MouseButtonDown final : public Event
    {
    public:
        EVENT(MouseButtonDown)
        explicit MouseButtonDown(MouseButton button) : Button(button)
        {
        }

        require_results std::string ToString() const override
        {
            return std::format("MouseButtonDown({})", static_cast<s32>(Button));
        }

        MouseButton Button{};
    };

    class MouseWheelMoved final : public Event
    {
    public:
        EVENT(MouseWheelMoved)
        explicit MouseWheelMoved(float x, float y) : X(x), Y(y)
        {
        }

        require_results std::array<f32, 2> Offset() const
        {
            return {X, Y};
        }

        require_results std::string ToString() const override
        {
            return std::format("MouseWheelMoved(X: {}, Y: {})", X, Y);
        }

        f32 X, Y{0.0f};
    };

}