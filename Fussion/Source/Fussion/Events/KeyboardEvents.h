#pragma once
#include "Event.h"
#include "Fussion/Input/Keys.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"

#include <format>
#include <magic_enum/magic_enum.hpp>

namespace Fussion {

class OnKeyDown final : public Event {
public:
    EVENT(OnKeyDown)

    explicit OnKeyDown(Keys key, KeyMods mods) : Key(key), Mods(mods) {}

    [[nodiscard]]
    virtual std::string ToString() const override
    {
        return std::format("OnKeyDown({})", magic_enum::enum_name(Key));
    }

    Keys Key{};
    KeyMods Mods{};
};

class OnKeyPressed final : public Event {
public:
    EVENT(OnKeyPressed)

    explicit OnKeyPressed(Keys key, KeyMods mods) : Key(key), Mods(mods) {}

    [[nodiscard]]
    virtual std::string ToString() const override
    {
        return std::format("OnKeyPressed({})", magic_enum::enum_name(Key));
    }

    Keys Key{};
    KeyMods Mods{};
};

class OnKeyReleased final : public Event {
public:
    EVENT(OnKeyReleased)

    explicit OnKeyReleased(Keys key, KeyMods mods) : Key(key), Mods(mods) {}

    [[nodiscard]]
    virtual std::string ToString() const override
    {
        return std::format("OnKeyReleased({})", magic_enum::enum_name(Key));
    }

    Keys Key{};
    KeyMods Mods{};
};

}

FSN_MAKE_FORMATTABLE(Fussion::OnKeyPressed, "OnKeyPressed({})", magic_enum::enum_name(v.Key))