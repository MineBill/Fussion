﻿#include "e5pch.h"
#include "Input.h"

#include <tracy/Tracy.hpp>

#include "Fussion/Events/KeyboardEvents.h"

namespace Fussion
{
    std::unordered_map<KeyboardKey, KeyState> g_Keys;
    std::unordered_map<MouseButton, KeyState> g_MouseButtons;

    bool Input::IsKeyDown(const KeyboardKey key)
    {
        return g_Keys[key] == KeyState::Pressed || g_Keys[key] == KeyState::HeldDown;
    }

    bool Input::IsKeyUp(const KeyboardKey key)
    {
        return g_Keys[key] == KeyState::Released || g_Keys[key] == KeyState::None;
    }

    bool Input::IsKeyPressed(const KeyboardKey key)
    {
        return g_Keys[key] == KeyState::Pressed;
    }

    bool Input::IsKeyReleased(const KeyboardKey key)
    {
        return g_Keys[key] == KeyState::Released;
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
    }

    void Input::Flush()
    {
        for(auto& [key, state] : g_Keys) {
            switch(state) {
            case KeyState::Pressed: {
                state = KeyState::HeldDown;
            } break;
            case KeyState::Released:{
                state = KeyState::None;
            } break;
            default: ;
            }
        }

        for(auto& [button, state] : g_MouseButtons) {
            switch(state) {
            case KeyState::Pressed: {
                state = KeyState::HeldDown;
            } break;
            case KeyState::Released:{
                state = KeyState::None;
            } break;
            default: ;
            }
        }
    }

    f32 Input::GetAxis(KeyboardKey positive, KeyboardKey negative) {
        return cast(f32, IsKeyDown(positive)) - cast(f32, IsKeyDown(negative));
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