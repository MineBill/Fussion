#pragma once
#include "Keys.h"
#include "Engin5/Events/Event.h"
#include "Engin5/Core/Types.h"

namespace Engin5
{
    enum class KeyState
    {
        None,
        Pressed,
        Released,
        HeldDown,
    };

    class Input
    {
        friend class Application;

    public:
        static bool IsKeyDown(KeyboardKey key);
        static bool IsKeyUp(KeyboardKey key);
        static bool IsKeyPressed(KeyboardKey key);
        static bool IsKeyReleased(KeyboardKey key);
        static f32 GetAxis(KeyboardKey positive, KeyboardKey negative);

        template<typename... K>
        static bool IsAnyKeyDown(const KeyboardKey key, K... keys)
        {
            if (IsKeyDown(key))
                return true;
            // @note Jesus fucking Christ, what the fuck is this
            if constexpr (sizeof...(keys) > 0)
                return IsAnyKeyDown(keys...);
            return false;
        }

    private:
        static void OnEvent(Event& event);
        static void Flush();
    };

}
