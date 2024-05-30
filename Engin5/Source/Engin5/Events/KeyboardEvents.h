#pragma once
#include "Event.h"
#include "Engin5/Input/Keys.h"
#include "Engin5/Core/Core.h"
#include "Engin5/Core/Types.h"

#include <format>
#include <magic_enum/magic_enum.hpp>

namespace Engin5
{

    class OnKeyDown final : public Event
    {
    public:
        EVENT(OnKeyDown)

        explicit OnKeyDown(KeyboardKey key) : Key(key)
        {
        }

        require_results std::string ToString() const override
        {
            return std::format("OnKeyDown({})", magic_enum::enum_name(Key));
        }

        KeyboardKey Key{};
    };

    class OnKeyPressed final : public Event
    {
    public:
        EVENT(OnKeyPressed)

        explicit OnKeyPressed(KeyboardKey key) : Key(key)
        {
        }

        require_results std::string ToString() const override
        {
            return std::format("OnKeyPressed({})", magic_enum::enum_name(Key));
        }

        KeyboardKey Key{};
    };

    class OnKeyReleased final : public Event
    {
    public:
        EVENT(OnKeyReleased)

        explicit OnKeyReleased(KeyboardKey key) : Key(key)
        {
        }

        require_results std::string ToString() const override
        {
            return std::format("OnKeyReleased({})", magic_enum::enum_name(Key));
        }

        KeyboardKey Key{};
    };

}