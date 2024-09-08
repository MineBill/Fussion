#include "FussionPCH.h"
#include "Input.h"

#include <tracy/Tracy.hpp>

#include "Fussion/Events/KeyboardEvents.h"

namespace Fussion {
    std::unordered_map<Keys, KeyState> g_Keys;
    std::unordered_map<MouseButton, KeyState> g_MouseButtons;

    struct State {
        Vector2 MousePosition{};
    } g_State;

    bool Input::is_key_down(Keys key)
    {
        return g_Keys[key] == KeyState::Pressed || g_Keys[key] == KeyState::HeldDown;
    }

    bool Input::is_key_up(Keys key)
    {
        return g_Keys[key] == KeyState::Released || g_Keys[key] == KeyState::None;
    }

    bool Input::is_key_pressed(Keys key)
    {
        return g_Keys[key] == KeyState::Pressed;
    }

    bool Input::is_key_released(Keys key)
    {
        return g_Keys[key] == KeyState::Released;
    }

    auto Input::mouse_position() -> Vector2
    {
        return g_State.MousePosition;
    }

    void Input::on_event(Event& event)
    {
        ZoneScoped;
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<OnKeyPressed>([](OnKeyPressed const& on_key_pressed) -> bool {
            g_Keys[on_key_pressed.key] = KeyState::Pressed;
            return false;
        });

        dispatcher.dispatch<OnKeyReleased>([](OnKeyReleased const& on_key_released) -> bool {
            g_Keys[on_key_released.key] = KeyState::Released;
            return false;
        });

        dispatcher.dispatch<MouseButtonPressed>([](MouseButtonPressed const& button_pressed) -> bool {
            g_MouseButtons[button_pressed.button] = KeyState::Pressed;
            return false;
        });

        dispatcher.dispatch<MouseButtonReleased>([](MouseButtonReleased const& button_released) -> bool {
            g_MouseButtons[button_released.button] = KeyState::Released;
            return false;
        });

        dispatcher.dispatch<MouseMoved>([](MouseMoved const& mouse_moved) {
            g_State.MousePosition = { mouse_moved.x, mouse_moved.y };
            return false;
        });
    }

    void Input::flush()
    {
        for (auto& [key, state] : g_Keys) {
            switch (state) {
            case KeyState::Pressed: {
                state = KeyState::HeldDown;
            }
            break;
            case KeyState::Released: {
                state = KeyState::None;
            }
            break;
            default: ;
            }
        }

        for (auto& [button, state] : g_MouseButtons) {
            switch (state) {
            case KeyState::Pressed: {
                state = KeyState::HeldDown;
            }
            break;
            case KeyState::Released: {
                state = KeyState::None;
            }
            break;
            default: ;
            }
        }
    }

    f32 Input::get_axis(Keys positive, Keys negative)
    {
        return CAST(f32, is_key_down(positive)) - CAST(f32, is_key_down(negative));
    }

    bool Input::is_mouse_button_down(MouseButton button)
    {
        return g_MouseButtons[button] == KeyState::Pressed || g_MouseButtons[button] == KeyState::HeldDown;
    }

    bool Input::is_mouse_button_up(MouseButton button)
    {
        return g_MouseButtons[button] == KeyState::Released || g_MouseButtons[button] == KeyState::None;
    }

    bool Input::is_mouse_button_pressed(MouseButton button)
    {
        return g_MouseButtons[button] == KeyState::Pressed;
    }

    bool Input::is_mouse_button_released(MouseButton button)
    {
        return g_MouseButtons[button] == KeyState::Released;
    }
}
