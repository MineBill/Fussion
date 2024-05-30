#include "e5pch.h"
#include "Input.h"

#include "Engin5/Events/KeyboardEvents.h"

namespace Engin5
{
    std::unordered_map<KeyboardKey, KeyState> g_Keys;

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
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<OnKeyPressed>([](OnKeyPressed& on_key_pressed) -> bool {
            g_Keys[on_key_pressed.Key] = KeyState::Pressed;
            return false;
        });

        dispatcher.Dispatch<OnKeyReleased>([](OnKeyReleased& on_key_released) -> bool {
            g_Keys[on_key_released.Key] = KeyState::Released;
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
    }

    f32 Input::GetAxis(KeyboardKey positive, KeyboardKey negative) {
        return cast(f32, IsKeyDown(positive)) - cast(f32, IsKeyDown(negative));
    }
}