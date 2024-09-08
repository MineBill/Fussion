#pragma once
#include "Core/BitFlags.h"
#include "Core/Types.h"
#include "Events/Event.h"
#include "Fussion/Math/Vector2.h"
#include <Fussion/Image.h>

namespace Fussion {
    enum class WindowFlag {
        Resizable = 1 << 0,
        Centered = 1 << 1,
        Decorated = 1 << 2,
    };

    DECLARE_FLAGS(WindowFlag, WindowFlags)

    DECLARE_OPERATORS_FOR_FLAGS(WindowFlags)

    struct WindowOptions {
        std::string initial_title{ "Window" };
        s32 initial_width{ 400 };
        s32 initial_height{ 400 };
        WindowFlags flags{};
    };

    enum class MouseMode {
        Unlocked,
        Locked,
        Confined,
    };

    class Window {
    public:
        virtual ~Window() = default;

        Vector2 size() const
        {
            return { CAST(f32, width()), CAST(f32, height()) };
        }

        virtual void set_mouse_mode(MouseMode mode) const = 0;

        static Window* create(WindowOptions const& options);

        virtual void update() = 0;

        virtual bool should_close() = 0;

        virtual void set_title(std::string const& title) = 0;

        virtual void on_event(EventFnType callback) = 0;

        virtual void* native_handle() const = 0;

        virtual void set_position(Vector2 position) const = 0;
        virtual auto position() const -> Vector2 = 0;

        virtual void set_icon(Image const& image) = 0;

        virtual u32 width() const = 0;
        virtual u32 height() const = 0;
    };
}
