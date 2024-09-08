#pragma once
#include "Fussion/Events/Event.h"

class Editor;

/// Base class for all editor windows.
class EditorWindow {
public:
    EditorWindow() = default;
    explicit EditorWindow(Editor* editor): m_editor{ editor } {}
    virtual ~EditorWindow() = default;

    virtual void on_start() {}
    virtual void on_draw() = 0;
    virtual void on_event([[maybe_unused]] Fussion::Event& event) {}

    /// Returns if the current window is focused or not, whatever that means for the
    /// current window.
    [[nodiscard]]
    bool is_focused() const { return m_is_focused; }

    /// Hide this editor window.
    void hide()
    {
        m_is_visible = false;
    }

    /// Show this editor window.
    void show()
    {
        m_is_visible = true;
    }

    void toggle()
    {
        if (m_is_visible) {
            hide();
        } else {
            show();
        }
    }

    /// Returns if this editor window is visible or not.
    [[nodiscard]]
    bool is_visible() const { return m_is_visible; }

protected:
    Editor* m_editor{};
    bool m_is_focused{ false };
    bool m_is_visible{ true };
};

#define EDITOR_WINDOW(TheName) \
    TheName() = default; \
    explicit TheName(Editor *editor): EditorWindow(editor) {}
