#pragma once
#include "Fussion/Events/Event.h"

class Editor;

/// Base class for all editor windows.
class EditorWindow {
public:
    EditorWindow() = default;
    explicit EditorWindow(Editor* editor)
        : m_Editor { editor }
    { }
    virtual ~EditorWindow() = default;

    virtual void OnStart() { }
    virtual void OnDraw() = 0;
    virtual void OnEvent([[maybe_unused]] Fussion::Event& event) { }

    /// Returns if the current window is focused or not, whatever that means for the
    /// current window.
    [[nodiscard]]
    bool IsFocused() const
    {
        return m_IsFocused;
    }

    /// Hide this editor window.
    void Hide()
    {
        m_IsVisible = false;
    }

    /// Show this editor window.
    void Show()
    {
        m_IsVisible = true;
    }

    void Toggle()
    {
        if (m_IsVisible) {
            Hide();
        } else {
            Show();
        }
    }

    /// Returns if this editor window is visible or not.
    [[nodiscard]]
    bool IsVisible() const
    {
        return m_IsVisible;
    }

protected:
    Editor* m_Editor {};
    bool m_IsFocused { false };
    bool m_IsVisible { true };
};

#define EDITOR_WINDOW(TheName)       \
    TheName() = default;             \
    explicit TheName(Editor* editor) \
        : EditorWindow(editor)       \
    { }
