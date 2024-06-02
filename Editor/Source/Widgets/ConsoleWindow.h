#pragma once
#include "EditorWindow.h"

class ConsoleWindow: public EditorWindow
{
public:
    WIDGET_CLASS(ConsoleWindow)

    void OnDraw() override;
};
