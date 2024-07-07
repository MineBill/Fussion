#pragma once
#include "Fussion/Scene/Component.h"
#include "Fussion/Scripting/ScriptAssembly.h"
#include "Fussion/Scripting/ScriptingEngine.h"

namespace Fussion
{
    class ScriptComponent: public Component
    {
    public:
        COMPONENT_DEFAULT(ScriptComponent)

        void OnUpdate(f32) override;

    private:
        ScriptClass* m_ScriptClass{nullptr};
        ScriptInstance* m_Instance{nullptr};
    };
}
