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

    class [[API]] Input {
        friend Application;

    public:
        static auto is_key_down(Keys key) -> bool;
        static auto is_key_up(Keys key) -> bool;
        static auto is_key_pressed(Keys key) -> bool;
        static auto is_key_released(Keys key) -> bool;
        static auto get_axis(Keys positive, Keys negative) -> f32;

        static auto is_mouse_button_down(MouseButton button) -> bool;
        static auto is_mouse_button_up(MouseButton button) -> bool;
        static auto is_mouse_button_pressed(MouseButton button) -> bool;
        static auto is_mouse_button_released(MouseButton button) -> bool;

        /// Gets the mouse position in window coordinates, relative to the
        /// top left of the window.
        static auto mouse_position() -> Vector2;

        template<std::same_as<Keys>... K>
        static auto is_any_key_down(Keys const key, K... keys) -> bool
        {
            if (is_key_down(key))
                return true;
            // @note Jesus fucking Christ, what the fuck is this
            if constexpr (sizeof...(keys) > 0)
                return is_any_key_down(keys...);
            return false;
        }

    private:
        static void on_event(Event& event);
        static void flush();
    };

}

namespace Fsn = Fussion;
