#pragma once
#include <Fussion/Core/BitFlags.h>
#include <Fussion/Core/Types.h>
#include <Fussion/Events/Event.h>
#include <Fussion/Image.h>
#include <Fussion/Math/Vector2.h>

namespace Fussion {
    enum class WindowFlag {
        Resizable = 1 << 0,
        Centered = 1 << 1,
        Decorated = 1 << 2,
    };

    DECLARE_FLAGS(WindowFlag, WindowFlags)

    DECLARE_OPERATORS_FOR_FLAGS(WindowFlags)

    struct WindowOptions {
        std::string InitialTitle { "Window" };
        s32 InitialWidth { 400 };
        s32 InitialHeight { 400 };
        WindowFlags Flags {};
    };

    enum class MouseMode {
        Unlocked,
        Locked,
        Confined,
    };

    class Window {
    public:
        virtual ~Window() = default;

        Vector2 Size() const
        {
            return { CAST(f32, Width()), CAST(f32, Height()) };
        }

        virtual void SetMouseMode(MouseMode mode) const = 0;

        static Window* Create(WindowOptions const& options);

        virtual void Update() = 0;

        virtual bool ShouldClose() = 0;

        virtual void SetEventCallback(EventFnType callback) = 0;

        virtual void* NativeHandle() const = 0;

        virtual void SetTitle(std::string const& title) = 0;
        virtual void SetPosition(Vector2 position) const = 0;
        virtual auto GetPosition() const -> Vector2 = 0;

        virtual void SetIcon(Image const& image) = 0;
        virtual void Maximize() = 0;

        [[nodiscard]] virtual u32 Width() const = 0;

        [[nodiscard]] virtual u32 Height() const = 0;
    };
}
