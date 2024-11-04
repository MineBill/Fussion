#pragma once
#include "EditorWindow.h"

class EngineInfoWindow : public EditorWindow {
public:
    EDITOR_WINDOW(EngineInfoWindow)

    virtual void OnDraw() override;
};
