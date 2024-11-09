#pragma once
#include "EditorWindow.h"

class EngineInfoWindow : public EditorWindow {
public:
    EDITOR_WINDOW(EngineInfoWindow)

    virtual void on_draw() override;
};
