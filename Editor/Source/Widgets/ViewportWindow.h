#pragma once
#include "EditorWindow.h"
#include "Engin5/Core/Types.h"

class ViewportWindow final: public EditorWindow
{
public:
    EDITOR_WINDOW(ViewportWindow)

    void OnDraw() override;

    Vector2 GetSize() const { return m_Size; }
private:
    Vector2 m_Size;
};
