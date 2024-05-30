#pragma once
#include "Core/BitFlags.h"
#include "Core/Types.h"
#include "Events/Event.h"

namespace Engin5
{
    enum class WindowFlag
    {
        Resizable,
    };

    DECLARE_FLAGS(WindowFlag, WindowFlags)
    DECLARE_OPERATORS_FOR_FLAGS(WindowFlags)

    struct WindowOptions
    {
        std::string InitialTitle{"Window"};
        s32 InitialWidth{400};
        s32 InitialHeight{400};
        WindowFlags Flags{};
    };

    class Window
    {
    public:
        virtual ~Window() = default;

        static Window* Create(const WindowOptions& options);

        virtual void Update() = 0;

        virtual bool ShouldClose() = 0;

        virtual void SetTitle(const std::string& title) = 0;

        virtual void OnEvent(EventFnType callback) = 0;

        virtual void* NativeHandle() const = 0;

        virtual void SetPosition(Vector2 position) const = 0;

        virtual u32 GetWidth() const = 0;
        virtual u32 GetHeight() const = 0;
    };
}