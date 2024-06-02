#include "e5pch.h"
#include "GlfwWindow.h"
#include "Engin5/Core/Core.h"
#include "Engin5/Events/ApplicationEvents.h"
#include "Engin5/Events/MouseEvents.h"
#include "Engin5/Events/KeyboardEvents.h"

#include "GLFW/glfw3.h"

namespace Engin5
{
    KeyboardKey GlfwKeyToEngin5(int key);
    MouseButton GlfwMouseButtonToEngin5(int glfw_mouse_button);

    Window* Window::Create(WindowOptions const& options)
    {
        return new GlfwWindow(options);
    }

    GlfwWindow::GlfwWindow(WindowOptions const& options)
        : m_Options(options)
    {
        if (glfwInit() != GLFW_TRUE) {
            exit(1);
        }

        if (options.Flags.Test(WindowFlag::Resizable)) {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow(options.InitialWidth, options.InitialHeight, options.InitialTitle.c_str(), nullptr, nullptr);

        if (options.Flags.Test(WindowFlag::Centered)) {
            auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            SetPosition({
                cast(f32, mode->width) / 2.0 - cast(f32, options.InitialWidth) / 2.0,
                cast(f32, mode->height) / 2.0 - cast(f32, options.InitialHeight) / 2.0
            });
        }

        glfwSetWindowUserPointer(m_Window, this);

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow *window, int width, int height) {
            auto me = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
            EASSERT(me != nullptr) // NOLINT(bugprone-lambda-function-name)
            me->m_Width = cast(u32, width);
            me->m_Height = cast(u32, height);
            auto event = WindowResized(width, height);
            me->m_EventCallback(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int glfw_key, int, int action, int) {
            auto me = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
            EASSERT(me != nullptr) // NOLINT(bugprone-lambda-function-name)

            auto key = GlfwKeyToEngin5(glfw_key);
            switch (action) {
            case GLFW_RELEASE: {
                OnKeyReleased event(key);
                me->m_EventCallback(event);
            } break;
            case GLFW_PRESS: {
                OnKeyPressed event(key);
                me->m_EventCallback(event);
            } break;
            case GLFW_REPEAT: {
                OnKeyDown event(key);
                me->m_EventCallback(event);
            } break;
            default:
                EASSERT(false, "Should never reach this assert")
            }
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, f64 x, f64 y) {
            auto me = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
            EASSERT(me != nullptr, "")

            auto rel_x = x - me->m_OldX;
            auto rel_y = y - me->m_OldY;
            me->m_OldX = cast(f32, x);
            me->m_OldY = cast(f32, y);

            auto event = MouseMoved(x, y, rel_x, rel_y);
            me->m_EventCallback(event);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int) {
            auto me = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
            EASSERT(me != nullptr, "")

            auto mouse_button = GlfwMouseButtonToEngin5(button);
            switch (action) {
            case GLFW_RELEASE: {
                MouseButtonReleased event(mouse_button);
                me->m_EventCallback(event);
            } break;
            case GLFW_PRESS: {
                MouseButtonPressed event(mouse_button);
                me->m_EventCallback(event);
            } break;
            case GLFW_REPEAT: {
                MouseButtonDown event(mouse_button);
                me->m_EventCallback(event);
            } break;
            default:
                EASSERT(false, "Should never reach this assert") // NOLINT(bugprone-lambda-function-name)
            }
        });

        glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow *window, int maximized) {
            auto me = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
            EASSERT(me != nullptr, "") // NOLINT(bugprone-lambda-function-name)

            if (maximized == 1) {
                me->m_IsMinimized = false;
                WindowMaximized event;
                me->m_EventCallback(event);
            }
        });

        glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, int minimized) {
            auto me = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
            EASSERT(me != nullptr, "") // NOLINT(bugprone-lambda-function-name)

            if (minimized == 1) {
                me->m_IsMinimized = true;
                WindowMinimized event;
                me->m_EventCallback(event);
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow *window, f64 x, f64 y) {
            auto me = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
            EASSERT(me != nullptr, "") // NOLINT(bugprone-lambda-function-name)

            MouseWheelMoved event(cast(f32, x), cast(f32, y));
            me->m_EventCallback(event);
        });
    }

    void GlfwWindow::Update()
    {
        glfwPollEvents();
    }

    bool GlfwWindow::ShouldClose()
    {
        return glfwWindowShouldClose(m_Window);
    }

    void GlfwWindow::SetTitle(const std::string& title)
    {
        if (m_IsMinimized) return;
        glfwSetWindowTitle(m_Window, title.c_str());
    }

    void GlfwWindow::OnEvent(const EventFnType callback)
    {
        m_EventCallback = callback;
    }

    void GlfwWindow::SetMouseMode(MouseMode mode) const
    {
        switch(mode) {
        case MouseMode::Unlocked: {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } break;
        case MouseMode::Locked: {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } break;
        case MouseMode::Confined: {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        } break;
        }
    }

    u32 GlfwWindow::GetHeight() const
    {
        s32 height;
        glfwGetFramebufferSize(m_Window, nullptr, &height);
        return cast(u32, height);
    }

    u32 GlfwWindow::GetWidth() const
    {
        s32 width;
        glfwGetFramebufferSize(m_Window, &width, nullptr);
        return cast(u32, width);
    }

    void* GlfwWindow::NativeHandle() const
    {
        return m_Window;
    }

    void GlfwWindow::SetPosition(Vector2 position) const
    {
        glfwSetWindowPos(m_Window, cast(s32, position.x), cast(s32, position.y));
    }

    KeyboardKey GlfwKeyToEngin5(int key)
    {
        switch (key) {
        case GLFW_KEY_SPACE         : return KeyboardKey::Space;
        case GLFW_KEY_APOSTROPHE    : return KeyboardKey::Apostrophe;
        case GLFW_KEY_COMMA         : return KeyboardKey::Comma;
        case GLFW_KEY_MINUS         : return KeyboardKey::Minus;
        case GLFW_KEY_PERIOD        : return KeyboardKey::Period;
        case GLFW_KEY_SLASH         : return KeyboardKey::Slash;
        case GLFW_KEY_0             : return KeyboardKey::Zero;
        case GLFW_KEY_1             : return KeyboardKey::One;
        case GLFW_KEY_2             : return KeyboardKey::Two;
        case GLFW_KEY_3             : return KeyboardKey::Three;
        case GLFW_KEY_4             : return KeyboardKey::Four;
        case GLFW_KEY_5             : return KeyboardKey::Five;
        case GLFW_KEY_6             : return KeyboardKey::Six;
        case GLFW_KEY_7             : return KeyboardKey::Seven;
        case GLFW_KEY_8             : return KeyboardKey::Eight;
        case GLFW_KEY_9             : return KeyboardKey::Nine;
        case GLFW_KEY_SEMICOLON     : return KeyboardKey::Semicolon;
        case GLFW_KEY_EQUAL         : return KeyboardKey::Equal;
        case GLFW_KEY_A             : return KeyboardKey::A;
        case GLFW_KEY_B             : return KeyboardKey::B;
        case GLFW_KEY_C             : return KeyboardKey::C;
        case GLFW_KEY_D             : return KeyboardKey::D;
        case GLFW_KEY_E             : return KeyboardKey::E;
        case GLFW_KEY_F             : return KeyboardKey::F;
        case GLFW_KEY_G             : return KeyboardKey::G;
        case GLFW_KEY_H             : return KeyboardKey::H;
        case GLFW_KEY_I             : return KeyboardKey::I;
        case GLFW_KEY_J             : return KeyboardKey::J;
        case GLFW_KEY_K             : return KeyboardKey::K;
        case GLFW_KEY_L             : return KeyboardKey::L;
        case GLFW_KEY_M             : return KeyboardKey::M;
        case GLFW_KEY_N             : return KeyboardKey::N;
        case GLFW_KEY_O             : return KeyboardKey::O;
        case GLFW_KEY_P             : return KeyboardKey::P;
        case GLFW_KEY_Q             : return KeyboardKey::Q;
        case GLFW_KEY_R             : return KeyboardKey::R;
        case GLFW_KEY_S             : return KeyboardKey::S;
        case GLFW_KEY_T             : return KeyboardKey::T;
        case GLFW_KEY_U             : return KeyboardKey::U;
        case GLFW_KEY_V             : return KeyboardKey::V;
        case GLFW_KEY_W             : return KeyboardKey::W;
        case GLFW_KEY_X             : return KeyboardKey::X;
        case GLFW_KEY_Y             : return KeyboardKey::Y;
        case GLFW_KEY_Z             : return KeyboardKey::Z;
        case GLFW_KEY_LEFT_BRACKET  : return KeyboardKey::LeftBracket;
        case GLFW_KEY_BACKSLASH     : return KeyboardKey::Backslash;
        case GLFW_KEY_RIGHT_BRACKET : return KeyboardKey::RightBracket;
        case GLFW_KEY_GRAVE_ACCENT  : return KeyboardKey::GraveAccent;
        case GLFW_KEY_WORLD_1       : return KeyboardKey::WORLD_1;
        case GLFW_KEY_WORLD_2       : return KeyboardKey::WORLD_2;
        case GLFW_KEY_ESCAPE        : return KeyboardKey::Escape;
        case GLFW_KEY_ENTER         : return KeyboardKey::Enter;
        case GLFW_KEY_TAB           : return KeyboardKey::Tab;
        case GLFW_KEY_BACKSPACE     : return KeyboardKey::Backspace;
        case GLFW_KEY_INSERT        : return KeyboardKey::Insert;
        case GLFW_KEY_DELETE        : return KeyboardKey::Delete;
        case GLFW_KEY_RIGHT         : return KeyboardKey::Right;
        case GLFW_KEY_LEFT          : return KeyboardKey::Left;
        case GLFW_KEY_DOWN          : return KeyboardKey::Down;
        case GLFW_KEY_UP            : return KeyboardKey::Up;
        case GLFW_KEY_PAGE_UP       : return KeyboardKey::PageUp;
        case GLFW_KEY_PAGE_DOWN     : return KeyboardKey::PageDown;
        case GLFW_KEY_HOME          : return KeyboardKey::Home;
        case GLFW_KEY_END           : return KeyboardKey::End;
        case GLFW_KEY_CAPS_LOCK     : return KeyboardKey::CapsLock;
        case GLFW_KEY_SCROLL_LOCK   : return KeyboardKey::ScrollLock;
        case GLFW_KEY_NUM_LOCK      : return KeyboardKey::NumLock;
        case GLFW_KEY_PRINT_SCREEN  : return KeyboardKey::PrintScreen;
        case GLFW_KEY_PAUSE         : return KeyboardKey::Pause;
        case GLFW_KEY_F1            : return KeyboardKey::F1;
        case GLFW_KEY_F2            : return KeyboardKey::F2;
        case GLFW_KEY_F3            : return KeyboardKey::F3;
        case GLFW_KEY_F4            : return KeyboardKey::F4;
        case GLFW_KEY_F5            : return KeyboardKey::F5;
        case GLFW_KEY_F6            : return KeyboardKey::F6;
        case GLFW_KEY_F7            : return KeyboardKey::F7;
        case GLFW_KEY_F8            : return KeyboardKey::F8;
        case GLFW_KEY_F9            : return KeyboardKey::F9;
        case GLFW_KEY_F10           : return KeyboardKey::F10;
        case GLFW_KEY_F11           : return KeyboardKey::F11;
        case GLFW_KEY_F12           : return KeyboardKey::F12;
        case GLFW_KEY_F13           : return KeyboardKey::F13;
        case GLFW_KEY_F14           : return KeyboardKey::F14;
        case GLFW_KEY_F15           : return KeyboardKey::F15;
        case GLFW_KEY_F16           : return KeyboardKey::F16;
        case GLFW_KEY_F17           : return KeyboardKey::F17;
        case GLFW_KEY_F18           : return KeyboardKey::F18;
        case GLFW_KEY_F19           : return KeyboardKey::F19;
        case GLFW_KEY_F20           : return KeyboardKey::F20;
        case GLFW_KEY_F21           : return KeyboardKey::F21;
        case GLFW_KEY_F22           : return KeyboardKey::F22;
        case GLFW_KEY_F23           : return KeyboardKey::F23;
        case GLFW_KEY_F24           : return KeyboardKey::F24;
        case GLFW_KEY_F25           : return KeyboardKey::F25;
        case GLFW_KEY_KP_0          : return KeyboardKey::Keypad0;
        case GLFW_KEY_KP_1          : return KeyboardKey::Keypad1;
        case GLFW_KEY_KP_2          : return KeyboardKey::Keypad2;
        case GLFW_KEY_KP_3          : return KeyboardKey::Keypad3;
        case GLFW_KEY_KP_4          : return KeyboardKey::Keypad4;
        case GLFW_KEY_KP_5          : return KeyboardKey::Keypad5;
        case GLFW_KEY_KP_6          : return KeyboardKey::Keypad6;
        case GLFW_KEY_KP_7          : return KeyboardKey::Keypad7;
        case GLFW_KEY_KP_8          : return KeyboardKey::Keypad8;
        case GLFW_KEY_KP_9          : return KeyboardKey::Keypad9;
        case GLFW_KEY_KP_DECIMAL    : return KeyboardKey::KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE     : return KeyboardKey::KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY   : return KeyboardKey::KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT   : return KeyboardKey::KeypadSubtract;
        case GLFW_KEY_KP_ADD        : return KeyboardKey::KeypadAdd;
        case GLFW_KEY_KP_ENTER      : return KeyboardKey::KeypadEnter;
        case GLFW_KEY_KP_EQUAL      : return KeyboardKey::KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT    : return KeyboardKey::LeftShift;
        case GLFW_KEY_LEFT_CONTROL  : return KeyboardKey::LeftControl;
        case GLFW_KEY_LEFT_ALT      : return KeyboardKey::LeftAlt;
        case GLFW_KEY_LEFT_SUPER    : return KeyboardKey::LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT   : return KeyboardKey::RightShift;
        case GLFW_KEY_RIGHT_CONTROL : return KeyboardKey::RightControl;
        case GLFW_KEY_RIGHT_ALT     : return KeyboardKey::RightAlt;
        case GLFW_KEY_RIGHT_SUPER   : return KeyboardKey::RightSuper;
        case GLFW_KEY_MENU          : return KeyboardKey::Menu;
        default                     : return KeyboardKey::None;
        }
            // clang-format on
    }

    MouseButton GlfwMouseButtonToEngin5(int glfw_mouse_button)
    {
        switch (glfw_mouse_button) {
        case GLFW_MOUSE_BUTTON_LEFT  : return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT : return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_4     : return MouseButton::Button4;
        case GLFW_MOUSE_BUTTON_5     : return MouseButton::Button5;
        case GLFW_MOUSE_BUTTON_6     : return MouseButton::Button6;
        case GLFW_MOUSE_BUTTON_7     : return MouseButton::Button7;
        case GLFW_MOUSE_BUTTON_8     : return MouseButton::Button8;
        default                      : return MouseButton::None;
        }
    }
}