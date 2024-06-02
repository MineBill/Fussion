#pragma once

class EditorLayer;
class EditorWindow
{
public:
    EditorWindow() = default;
    EditorWindow(EditorLayer *editor): m_Editor{editor} {}
    virtual void OnDraw() = 0;

    bool IsFocused() const { return m_IsFocused; }

protected:
    EditorLayer *m_Editor{};
    bool m_IsFocused{false};
};

#define WIDGET_CLASS(TheName) \
    TheName() = default; \
    TheName(EditorLayer *editor): EditorWindow(editor) {}
