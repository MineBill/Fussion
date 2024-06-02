#pragma once
#include "EditorWindow.h"

class InspectorWindow: public EditorWindow
{
public:
    WIDGET_CLASS(InspectorWindow)

    void OnDraw() override;
};
