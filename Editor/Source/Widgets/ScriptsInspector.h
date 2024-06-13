#pragma once
#include "EditorWindow.h"
#include "Fussion/Scripting/ScriptAssembly.h"

class ScriptsInspector: public EditorWindow
{
public:
    EDITOR_WINDOW(ScriptsInspector)

    void OnDraw() override;

private:
    Fussion::ScriptClass* m_SelectedClass{nullptr};
};
