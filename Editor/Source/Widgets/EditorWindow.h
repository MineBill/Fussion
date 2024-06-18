#pragma once

class Editor;

/// Base class for all editor windows.
class EditorWindow
{
public:
    EditorWindow() = default;
    EditorWindow(Editor *editor): m_Editor{editor} {}
    virtual ~EditorWindow() = default;

    virtual void OnStart() {}
    virtual void OnDraw() = 0;

    /// Returns if the current window is focused or not, whatever that means for the
    /// current window.
    [[nodiscard]] bool IsFocused() const { return m_IsFocused; }

    void Hide()
    {
        m_IsVisible = false;
    }

    void Show()
    {
        m_IsVisible = true;
    }

    [[nodiscard]] bool IsVisible() const { return m_IsVisible; }

protected:
    Editor *m_Editor{};
    bool m_IsFocused{false};
    bool m_IsVisible{true};
};

#define EDITOR_WINDOW(TheName) \
    TheName() = default; \
    TheName(Editor *editor): EditorWindow(editor) {}
