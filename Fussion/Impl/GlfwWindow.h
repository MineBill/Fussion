#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Window.h"
#include "GLFW/glfw3.h"

namespace Fussion {
    class GlfwWindow : public Window {
    public:
        explicit GlfwWindow(WindowOptions const& options);
        virtual ~GlfwWindow() override;

        virtual void Update() override;
        virtual bool ShouldClose() override;
        virtual void SetTitle(std::string const& title) override;
        virtual void SetEventCallback(EventFnType callback) override;

        virtual void SetMouseMode(MouseMode mode) const override;

        virtual u32 Height() const override;
        virtual u32 Width() const override;

        virtual void SetPosition(Vector2 position) const override;
        virtual auto GetPosition() const -> Vector2 override;

        virtual void SetIcon(Image const& image) override;
        virtual void Maximize() override;

        virtual void* NativeHandle() const override;

    private:
        bool m_IsMinimized { false };

        WindowOptions m_Options {};
        GLFWwindow* m_Window {};
        EventFnType m_EventCallback {};

        f32 m_OldX {}, m_OldY {};
        u32 m_Width {}, m_Height {};
    };
}
