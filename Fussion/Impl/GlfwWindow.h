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

        virtual void Update() override;
        virtual bool ShouldClose() override;
        virtual void SetTitle(std::string const& title) override;
        virtual void OnEvent(EventFnType callback) override;

        virtual void SetMouseMode(MouseMode mode) const override;

        virtual u32 GetHeight() const override;
        virtual u32 GetWidth() const override;

        virtual void SetPosition(Vector2 position) const override;

        virtual void* NativeHandle() const override;

    private:
        bool m_IsMinimized{false};

        WindowOptions m_Options{};
        GLFWwindow* m_Window{};
        EventFnType m_EventCallback{};

        f32 m_OldX{}, m_OldY{};
        u32 m_Width{}, m_Height{};
    };
}