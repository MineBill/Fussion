#pragma once
#include "Keys.h"
#include "Fussion/Events/Event.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Events/MouseEvents.h"

namespace Fussion {
class Application;

enum class KeyState {
    None,
    Pressed,
    Released,
    HeldDown,
};

class Input {
    friend Application;

public:
    static bool IsKeyDown(Keys key);
    static bool IsKeyUp(Keys key);
    static bool IsKeyPressed(Keys key);
    static bool IsKeyReleased(Keys key);
    static f32 GetAxis(Keys positive, Keys negative);

    static bool IsMouseButtonDown(MouseButton button);
    static bool IsMouseButtonUp(MouseButton button);
    static bool IsMouseButtonPressed(MouseButton button);
    static bool IsMouseButtonReleased(MouseButton button);

    template<typename... K>
    static bool IsAnyKeyDown(Keys const key, K... keys)
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

namespace Fsn = Fussion;
