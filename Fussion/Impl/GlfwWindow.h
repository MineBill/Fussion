#pragma once
#include "Fussion/Window.h"
#include "Fussion/Core/Types.h"
#include "GLFW/glfw3.h"

namespace Fussion
{
    class GlfwWindow: public Window
    {
    public:
        explicit GlfwWindow(WindowOptions const& options);
        virtual ~GlfwWindow() override;

        virtual void update() override;
        virtual bool should_close() override;
        virtual void set_title(std::string const& title) override;
        virtual void on_event(EventFnType callback) override;

        virtual void set_mouse_mode(MouseMode mode) const override;

        virtual u32 height() const override;
        virtual u32 width() const override;

        virtual void set_position(Vector2 position) const override;
        virtual auto position() const -> Vector2 override;

        virtual void set_icon(Image const& image) override;

        virtual void* native_handle() const override;

    private:
        bool m_IsMinimized{false};

        WindowOptions m_Options{};
        GLFWwindow* m_Window{};
        EventFnType m_EventCallback{};

        f32 m_OldX{}, m_OldY{};
        u32 m_Width{}, m_Height{};
    };
}