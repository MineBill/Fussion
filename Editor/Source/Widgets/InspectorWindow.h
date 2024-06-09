#pragma once
#include "EditorWindow.h"
#include "Engin5/Scene/Entity.h"

class InspectorWindow final: public EditorWindow
{
public:
    EDITOR_WINDOW(InspectorWindow)

    void OnStart() override;
    void OnDraw() override;

private:
    void DrawEntity(Engin5::Entity& e);

    void DrawComponent();
};
