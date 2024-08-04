#include "e5pch.h"
#include "Input.h"

#include <tracy/Tracy.hpp>

#include "Fussion/Events/KeyboardEvents.h"

namespace Fussion {
    std::unordered_map<Keys, KeyState> g_Keys;
    std::unordered_map<MouseButton, KeyState> g_MouseButtons;

    struct State {
        Vector2 MousePosition{};
    } g_State;

    bool Input::IsKeyDown(Keys key)
    {
        return g_Keys[key] == KeyState::Pressed || g_Keys[key] == KeyState::HeldDown;
    }

    bool Input::IsKeyUp(Keys key)
    {
        return g_Keys[key] == KeyState::Released || g_Keys[key] == KeyState::None;
    }

    bool Input::IsKeyPressed(Keys key)
    {
        return g_Keys[key] == KeyState::Pressed;
    }

    bool Input::IsKeyReleased(Keys key)
    {
        return g_Keys[key] == KeyState::Released;
    }

    auto Input::GetMousePosition() -> Vector2
    {
        return g_State.MousePosition;
    }

    void Input::OnEvent(Event& event)
    {
        ZoneScoped;
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<OnKeyPressed>([](OnKeyPressed const& on_key_pressed) -> bool {
            g_Keys[on_key_pressed.Key] = KeyState::Pressed;
            return false;
        });

        dispatcher.Dispatch<OnKeyReleased>([](OnKeyReleased const& on_key_released) -> bool {
            g_Keys[on_key_released.Key] = KeyState::Released;
            return false;
        });

        dispatcher.Dispatch<MouseButtonPressed>([](MouseButtonPressed const& button_pressed) -> bool {
            g_MouseButtons[button_pressed.Button] = KeyState::Pressed;
            return false;
        });

        dispatcher.Dispatch<MouseButtonReleased>([](MouseButtonReleased const& button_released) -> bool {
            g_MouseButtons[button_released.Button] = KeyState::Released;
            return false;
        });

        dispatcher.Dispatch<MouseMoved>([](MouseMoved const& mouse_moved) {
            g_State.MousePosition = { mouse_moved.X, mouse_moved.Y };
            return false;
        });
    }

    void Input::Flush()
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

    f32 Input::GetAxis(Keys positive, Keys negative)
    {
        return CAST(f32, IsKeyDown(positive)) - CAST(f32, IsKeyDown(negative));
    }

    bool Input::IsMouseButtonDown(MouseButton button)
    {
        return g_MouseButtons[button] == KeyState::Pressed || g_MouseButtons[button] == KeyState::HeldDown;
    }

    bool Input::IsMouseButtonUp(MouseButton button)
    {
        return g_MouseButtons[button] == KeyState::Released || g_MouseButtons[button] == KeyState::None;
    }

    bool Input::IsMouseButtonPressed(MouseButton button)
    {
        return g_MouseButtons[button] == KeyState::Pressed;
    }

    bool Input::IsMouseButtonReleased(MouseButton button)
    {
        return g_MouseButtons[button] == KeyState::Released;
    }
}
