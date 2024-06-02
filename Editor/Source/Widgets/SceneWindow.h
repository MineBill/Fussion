#pragma once
#include "EditorWindow.h"

class SceneWindow: public EditorWindow
{
public:
    WIDGET_CLASS(SceneWindow)

    void OnDraw() override;
};
