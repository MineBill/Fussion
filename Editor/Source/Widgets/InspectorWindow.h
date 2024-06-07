#pragma once
#include "EditorWindow.h"
#include "Engin5/Scene/Entity.h"

class InspectorWindow final: public EditorWindow
{
public:
    EDITOR_WINDOW(InspectorWindow)

    void OnDraw() override;

private:
    void DrawEntity(Engin5::Entity& e);

    void DrawComponent();
};
