#pragma once
#include "EditorWindow.h"
#include "Engin5/Scripting/ScriptAssembly.h"

class ScriptsInspector: public EditorWindow
{
public:
    EDITOR_WINDOW(ScriptsInspector)

    void OnDraw() override;

private:
    Engin5::ScriptClass* m_SelectedClass{nullptr};
};
