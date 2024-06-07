#pragma once

class Editor;
class EditorWindow
{
public:
    EditorWindow() = default;
    EditorWindow(Editor *editor): m_Editor{editor} {}
    virtual ~EditorWindow() = default;

    virtual void OnStart() {}
    virtual void OnDraw() = 0;

    bool IsFocused() const { return m_IsFocused; }

protected:
    Editor *m_Editor{};
    bool m_IsFocused{false};
};

#define EDITOR_WINDOW(TheName) \
    TheName() = default; \
    TheName(Editor *editor): EditorWindow(editor) {}
