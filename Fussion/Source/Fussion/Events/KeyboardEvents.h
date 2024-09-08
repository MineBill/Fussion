#pragma once
#include "Event.h"
#include "Fussion/Input/Keys.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"

#include <magic_enum/magic_enum.hpp>

namespace Fussion {
    class OnKeyDown final : public Event {
    public:
        EVENT(OnKeyDown)

        explicit OnKeyDown(Keys key, KeyMods mods) : key(key), mods(mods) {}

        Keys key{};
        KeyMods mods{};
    };

    class OnKeyPressed final : public Event {
    public:
        EVENT(OnKeyPressed)

        explicit OnKeyPressed(Keys key, KeyMods mods) : key(key), mods(mods) {}

        Keys key{};
        KeyMods mods{};
    };

    class OnKeyReleased final : public Event {
    public:
        EVENT(OnKeyReleased)

        explicit OnKeyReleased(Keys key, KeyMods mods) : key(key), mods(mods) {}

        Keys key{};
        KeyMods mods{};
    };
}

FSN_MAKE_FORMATTABLE(Fussion::OnKeyPressed, "OnKeyPressed({})", magic_enum::enum_name(v.key))
