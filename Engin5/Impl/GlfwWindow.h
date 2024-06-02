#pragma once
#include "Engin5/Window.h"
#include "Engin5/Core/Types.h"
#include "GLFW/glfw3.h"

namespace Engin5
{
    class GlfwWindow: public Window
    {
    public:
        GlfwWindow(const WindowOptions& options);

        void Update() override;
        bool ShouldClose() override;
        void SetTitle(const std::string& title) override;
        void OnEvent(EventFnType callback) override;

        void SetMouseMode(MouseMode mode) const override;

        u32 GetHeight() const override;
        u32 GetWidth() const override;

        void SetPosition(Vector2 position) const override;

        void* NativeHandle() const override;

    private:
        bool m_IsMinimized{false};

        WindowOptions m_Options{};
        GLFWwindow* m_Window{};
        EventFnType m_EventCallback{};

        f32 m_OldX{}, m_OldY{};
        u32 m_Width{}, m_Height{};
    };
}