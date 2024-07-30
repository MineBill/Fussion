#pragma once
#include "EditorWindow.h"
#include "Fussion/Scripting/ScriptAssembly.h"

class ScriptsInspector final : public EditorWindow
{
public:
    EDITOR_WINDOW(ScriptsInspector)

    virtual void OnDraw() override;

private:
    Fussion::ScriptClass* m_SelectedClass{nullptr};
};
