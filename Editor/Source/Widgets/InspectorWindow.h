#pragma once
#include "EditorWindow.h"
#include "Fussion/Scene/Entity.h"

class InspectorWindow final: public EditorWindow
{
public:
    EDITOR_WINDOW(InspectorWindow)

    void OnStart() override;
    void OnDraw() override;

private:
    void DrawEntity(Fussion::Entity& e);

    void DrawComponent();
};
