#include "FussionPCH.h"
#include "GlfwWindow.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Events/ApplicationEvents.h"
#include "Fussion/Events/MouseEvents.h"
#include "Fussion/Events/KeyboardEvents.h"

#include "GLFW/glfw3.h"
#include "OS/System.h"

#include <tracy/Tracy.hpp>

#ifdef OS_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include "Glfw/glfw3native.h"
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif

namespace Fussion {
    Keys GlfwKeyToFussion(int key);
    MouseButton GlfwMouseButtonToFussion(int glfw_mouse_button);

    Window* Window::create(WindowOptions const& options)
    {
        return new GlfwWindow(options);
    }

    GlfwWindow::GlfwWindow(WindowOptions const& options)
        : m_options(options)
    {
        if (glfwInit() != GLFW_TRUE) {
            exit(1);
        }

        if (options.flags.test(WindowFlag::Resizable)) {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        }

        if (options.flags.test(WindowFlag::Decorated)) {
            glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
        } else {
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window = glfwCreateWindow(options.initial_width, options.initial_height, options.initial_title.c_str(), nullptr, nullptr);

        if (options.flags.test(WindowFlag::Centered)) {
            auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            set_position({
                CAST(f32, mode->width) / 2.0 - CAST(f32, options.initial_width) / 2.0,
                CAST(f32, mode->height) / 2.0 - CAST(f32, options.initial_height) / 2.0
            });
        }

#if OS_WINDOWS
        if (System::prefers_dark()) {
            constexpr auto dwmwa_use_immersive_dark_mode = 20;
            BOOL value = TRUE;
            (void)DwmSetWindowAttribute(glfwGetWin32Window(m_window), dwmwa_use_immersive_dark_mode, &value, sizeof(value));
        }
#endif

        if (glfwRawMouseMotionSupported()) {
            LOG_INFO("Enabling raw mouse motion");
            glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        glfwSetWindowUserPointer(m_window, this);

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
            auto me = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            VERIFY(me != nullptr) // NOLINT(bugprone-lambda-function-name)
            auto event = WindowCloseRequest();
            me->m_event_callback(event);
        });

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            auto me = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            VERIFY(me != nullptr) // NOLINT(bugprone-lambda-function-name)
            me->m_width = CAST(u32, width);
            me->m_height = CAST(u32, height);
            auto event = WindowResized(width, height);
            me->m_event_callback(event);
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int glfw_key, int, int action, int mods) {
            auto me = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            VERIFY(me != nullptr) // NOLINT(bugprone-lambda-function-name)

            KeyMods key_mods{};
            if ((mods & GLFW_MOD_ALT) != 0) {
                key_mods |= KeyMod::Alt;
            }
            if ((mods & GLFW_MOD_SHIFT) != 0) {
                key_mods |= KeyMod::Shift;
            }
            if ((mods & GLFW_MOD_SUPER) != 0) {
                key_mods |= KeyMod::Super;
            }
            if ((mods & GLFW_MOD_CONTROL) != 0) {
                key_mods |= KeyMod::Control;
            }
            if ((mods & GLFW_MOD_CAPS_LOCK) != 0) {
                key_mods |= KeyMod::CapsLock;
            }

            auto key = GlfwKeyToFussion(glfw_key);
            switch (action) {
            case GLFW_RELEASE: {
                OnKeyReleased event(key, key_mods);
                me->m_event_callback(event);
            }
            break;
            case GLFW_PRESS: {
                OnKeyPressed event(key, key_mods);
                me->m_event_callback(event);
            }
            break;
            case GLFW_REPEAT: {
                OnKeyDown event(key, key_mods);
                me->m_event_callback(event);
            }
            break;
            default:
                UNREACHABLE;
            }
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, f64 x, f64 y) {
            auto me = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            VERIFY(me != nullptr, "")

            auto rel_x = x - me->m_old_x;
            auto rel_y = y - me->m_old_y;
            me->m_old_x = CAST(f32, x);
            me->m_old_y = CAST(f32, y);

            auto event = MouseMoved(x, y, rel_x, rel_y);
            me->m_event_callback(event);
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int) {
            auto me = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            VERIFY(me != nullptr, "")

            auto mouse_button = GlfwMouseButtonToFussion(button);
            switch (action) {
            case GLFW_RELEASE: {
                MouseButtonReleased event(mouse_button);
                me->m_event_callback(event);
            }
            break;
            case GLFW_PRESS: {
                MouseButtonPressed event(mouse_button);
                me->m_event_callback(event);
            }
            break;
            case GLFW_REPEAT: {
                MouseButtonDown event(mouse_button);
                me->m_event_callback(event);
            }
            break;
            default:
                UNREACHABLE;
            }
        });

        glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow* window, int maximized) {
            auto me = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            VERIFY(me != nullptr, "") // NOLINT(bugprone-lambda-function-name)

            if (maximized == 1) {
                me->m_is_minimized = false;
                WindowMaximized event;
                me->m_event_callback(event);
            }
        });

        glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* window, int minimized) {
            auto me = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            VERIFY(me != nullptr, "") // NOLINT(bugprone-lambda-function-name)

            if (minimized == 1) {
                me->m_is_minimized = true;
                WindowMinimized event;
                me->m_event_callback(event);
            }
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow* window, f64 x, f64 y) {
            auto me = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
            VERIFY(me != nullptr, "") // NOLINT(bugprone-lambda-function-name)

            MouseWheelMoved event(CAST(f32, x), CAST(f32, y));
            me->m_event_callback(event);
        });
    }

    GlfwWindow::~GlfwWindow()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void GlfwWindow::update()
    {
        ZoneScopedN("Window Update");
        glfwPollEvents();
    }

    bool GlfwWindow::should_close()
    {
        return glfwWindowShouldClose(m_window);
    }

    void GlfwWindow::set_title(std::string const& title)
    {
        if (m_is_minimized)
            return;
        glfwSetWindowTitle(m_window, title.c_str());
    }

    void GlfwWindow::on_event(EventFnType const callback)
    {
        m_event_callback = callback;
    }

    void GlfwWindow::set_mouse_mode(MouseMode mode) const
    {
        switch (mode) {
        case MouseMode::Unlocked: {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        break;
        case MouseMode::Locked: {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        break;
        case MouseMode::Confined: {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        }
        break;
        }
    }

    u32 GlfwWindow::height() const
    {
        s32 height;
        glfwGetFramebufferSize(m_window, nullptr, &height);
        return CAST(u32, height);
    }

    u32 GlfwWindow::width() const
    {
        s32 width;
        glfwGetFramebufferSize(m_window, &width, nullptr);
        return CAST(u32, width);
    }

    void* GlfwWindow::native_handle() const
    {
        return m_window;
    }

    void GlfwWindow::set_position(Vector2 position) const
    {
        glfwSetWindowPos(m_window, CAST(s32, position.x), CAST(s32, position.y));
    }

    auto GlfwWindow::position() const -> Vector2
    {
        s32 x, y;
        glfwGetWindowPos(m_window, &x, &y);
        return { x, y };
    }

    void GlfwWindow::set_icon(Image const& image)
    {
        GLFWimage glfw_image;
        glfw_image.pixels = const_cast<Image&>(image).data.data();
        glfw_image.height = image.height;
        glfw_image.width = image.width;

        glfwSetWindowIcon(m_window, 1, &glfw_image);
    }

    Keys GlfwKeyToFussion(int key)
    {
        switch (key) {
        case GLFW_KEY_SPACE:
            return Keys::Space;
        case GLFW_KEY_APOSTROPHE:
            return Keys::Apostrophe;
        case GLFW_KEY_COMMA:
            return Keys::Comma;
        case GLFW_KEY_MINUS:
            return Keys::Minus;
        case GLFW_KEY_PERIOD:
            return Keys::Period;
        case GLFW_KEY_SLASH:
            return Keys::Slash;
        case GLFW_KEY_0:
            return Keys::Zero;
        case GLFW_KEY_1:
            return Keys::One;
        case GLFW_KEY_2:
            return Keys::Two;
        case GLFW_KEY_3:
            return Keys::Three;
        case GLFW_KEY_4:
            return Keys::Four;
        case GLFW_KEY_5:
            return Keys::Five;
        case GLFW_KEY_6:
            return Keys::Six;
        case GLFW_KEY_7:
            return Keys::Seven;
        case GLFW_KEY_8:
            return Keys::Eight;
        case GLFW_KEY_9:
            return Keys::Nine;
        case GLFW_KEY_SEMICOLON:
            return Keys::Semicolon;
        case GLFW_KEY_EQUAL:
            return Keys::Equal;
        case GLFW_KEY_A:
            return Keys::A;
        case GLFW_KEY_B:
            return Keys::B;
        case GLFW_KEY_C:
            return Keys::C;
        case GLFW_KEY_D:
            return Keys::D;
        case GLFW_KEY_E:
            return Keys::E;
        case GLFW_KEY_F:
            return Keys::F;
        case GLFW_KEY_G:
            return Keys::G;
        case GLFW_KEY_H:
            return Keys::H;
        case GLFW_KEY_I:
            return Keys::I;
        case GLFW_KEY_J:
            return Keys::J;
        case GLFW_KEY_K:
            return Keys::K;
        case GLFW_KEY_L:
            return Keys::L;
        case GLFW_KEY_M:
            return Keys::M;
        case GLFW_KEY_N:
            return Keys::N;
        case GLFW_KEY_O:
            return Keys::O;
        case GLFW_KEY_P:
            return Keys::P;
        case GLFW_KEY_Q:
            return Keys::Q;
        case GLFW_KEY_R:
            return Keys::R;
        case GLFW_KEY_S:
            return Keys::S;
        case GLFW_KEY_T:
            return Keys::T;
        case GLFW_KEY_U:
            return Keys::U;
        case GLFW_KEY_V:
            return Keys::V;
        case GLFW_KEY_W:
            return Keys::W;
        case GLFW_KEY_X:
            return Keys::X;
        case GLFW_KEY_Y:
            return Keys::Y;
        case GLFW_KEY_Z:
            return Keys::Z;
        case GLFW_KEY_LEFT_BRACKET:
            return Keys::LeftBracket;
        case GLFW_KEY_BACKSLASH:
            return Keys::Backslash;
        case GLFW_KEY_RIGHT_BRACKET:
            return Keys::RightBracket;
        case GLFW_KEY_GRAVE_ACCENT:
            return Keys::GraveAccent;
        case GLFW_KEY_WORLD_1:
            return Keys::WORLD_1;
        case GLFW_KEY_WORLD_2:
            return Keys::WORLD_2;
        case GLFW_KEY_ESCAPE:
            return Keys::Escape;
        case GLFW_KEY_ENTER:
            return Keys::Enter;
        case GLFW_KEY_TAB:
            return Keys::Tab;
        case GLFW_KEY_BACKSPACE:
            return Keys::Backspace;
        case GLFW_KEY_INSERT:
            return Keys::Insert;
        case GLFW_KEY_DELETE:
            return Keys::Delete;
        case GLFW_KEY_RIGHT:
            return Keys::Right;
        case GLFW_KEY_LEFT:
            return Keys::Left;
        case GLFW_KEY_DOWN:
            return Keys::Down;
        case GLFW_KEY_UP:
            return Keys::Up;
        case GLFW_KEY_PAGE_UP:
            return Keys::PageUp;
        case GLFW_KEY_PAGE_DOWN:
            return Keys::PageDown;
        case GLFW_KEY_HOME:
            return Keys::Home;
        case GLFW_KEY_END:
            return Keys::End;
        case GLFW_KEY_CAPS_LOCK:
            return Keys::CapsLock;
        case GLFW_KEY_SCROLL_LOCK:
            return Keys::ScrollLock;
        case GLFW_KEY_NUM_LOCK:
            return Keys::NumLock;
        case GLFW_KEY_PRINT_SCREEN:
            return Keys::PrintScreen;
        case GLFW_KEY_PAUSE:
            return Keys::Pause;
        case GLFW_KEY_F1:
            return Keys::F1;
        case GLFW_KEY_F2:
            return Keys::F2;
        case GLFW_KEY_F3:
            return Keys::F3;
        case GLFW_KEY_F4:
            return Keys::F4;
        case GLFW_KEY_F5:
            return Keys::F5;
        case GLFW_KEY_F6:
            return Keys::F6;
        case GLFW_KEY_F7:
            return Keys::F7;
        case GLFW_KEY_F8:
            return Keys::F8;
        case GLFW_KEY_F9:
            return Keys::F9;
        case GLFW_KEY_F10:
            return Keys::F10;
        case GLFW_KEY_F11:
            return Keys::F11;
        case GLFW_KEY_F12:
            return Keys::F12;
        case GLFW_KEY_F13:
            return Keys::F13;
        case GLFW_KEY_F14:
            return Keys::F14;
        case GLFW_KEY_F15:
            return Keys::F15;
        case GLFW_KEY_F16:
            return Keys::F16;
        case GLFW_KEY_F17:
            return Keys::F17;
        case GLFW_KEY_F18:
            return Keys::F18;
        case GLFW_KEY_F19:
            return Keys::F19;
        case GLFW_KEY_F20:
            return Keys::F20;
        case GLFW_KEY_F21:
            return Keys::F21;
        case GLFW_KEY_F22:
            return Keys::F22;
        case GLFW_KEY_F23:
            return Keys::F23;
        case GLFW_KEY_F24:
            return Keys::F24;
        case GLFW_KEY_F25:
            return Keys::F25;
        case GLFW_KEY_KP_0:
            return Keys::Keypad0;
        case GLFW_KEY_KP_1:
            return Keys::Keypad1;
        case GLFW_KEY_KP_2:
            return Keys::Keypad2;
        case GLFW_KEY_KP_3:
            return Keys::Keypad3;
        case GLFW_KEY_KP_4:
            return Keys::Keypad4;
        case GLFW_KEY_KP_5:
            return Keys::Keypad5;
        case GLFW_KEY_KP_6:
            return Keys::Keypad6;
        case GLFW_KEY_KP_7:
            return Keys::Keypad7;
        case GLFW_KEY_KP_8:
            return Keys::Keypad8;
        case GLFW_KEY_KP_9:
            return Keys::Keypad9;
        case GLFW_KEY_KP_DECIMAL:
            return Keys::KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE:
            return Keys::KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY:
            return Keys::KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT:
            return Keys::KeypadSubtract;
        case GLFW_KEY_KP_ADD:
            return Keys::KeypadAdd;
        case GLFW_KEY_KP_ENTER:
            return Keys::KeypadEnter;
        case GLFW_KEY_KP_EQUAL:
            return Keys::KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT:
            return Keys::LeftShift;
        case GLFW_KEY_LEFT_CONTROL:
            return Keys::LeftControl;
        case GLFW_KEY_LEFT_ALT:
            return Keys::LeftAlt;
        case GLFW_KEY_LEFT_SUPER:
            return Keys::LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT:
            return Keys::RightShift;
        case GLFW_KEY_RIGHT_CONTROL:
            return Keys::RightControl;
        case GLFW_KEY_RIGHT_ALT:
            return Keys::RightAlt;
        case GLFW_KEY_RIGHT_SUPER:
            return Keys::RightSuper;
        case GLFW_KEY_MENU:
            return Keys::Menu;
        default:
            return Keys::None;
        }
        // clang-format on
    }

    MouseButton GlfwMouseButtonToFussion(int glfw_mouse_button)
    {
        switch (glfw_mouse_button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_4:
            return MouseButton::Button4;
        case GLFW_MOUSE_BUTTON_5:
            return MouseButton::Button5;
        case GLFW_MOUSE_BUTTON_6:
            return MouseButton::Button6;
        case GLFW_MOUSE_BUTTON_7:
            return MouseButton::Button7;
        case GLFW_MOUSE_BUTTON_8:
            return MouseButton::Button8;
        default:
            return MouseButton::None;
        }
    }
}
