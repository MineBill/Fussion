#pragma once
#include "Keys.h"
#include "Fussion/Events/Event.h"
#include "Fussion/Core/Types.h"
#include "Fussion/Events/MouseEvents.h"
#include <Fussion/Math/Vector2.h>

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
        static auto IsKeyDown(Keys key) -> bool;
        static auto IsKeyUp(Keys key) -> bool;
        static auto IsKeyPressed(Keys key) -> bool;
        static auto IsKeyReleased(Keys key) -> bool;
        static auto GetAxis(Keys positive, Keys negative) -> f32;

        static auto IsMouseButtonDown(MouseButton button) -> bool;
        static auto IsMouseButtonUp(MouseButton button) -> bool;
        static auto IsMouseButtonPressed(MouseButton button) -> bool;
        static auto IsMouseButtonReleased(MouseButton button) -> bool;

        /// Gets the mouse position in window coordinates, relative to the
        /// top left of the window.
        static auto GetMousePosition() -> Vector2;

        template<std::same_as<Keys>... K>
        static auto IsAnyKeyDown(Keys const key, K... keys) -> bool
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
