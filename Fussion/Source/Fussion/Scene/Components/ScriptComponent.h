#pragma once
#include "Fussion/Scene/Component.h"
#include "Scripting/ScriptAssembly.h"
#include "Scripting/ScriptingEngine.h"

namespace Fussion
{
    class ScriptComponent: public Component
    {
    public:
        ScriptComponent() = default;
        COMPONENT(ScriptComponent)

        void OnUpdate(f32) override;

    private:
        ScriptClass* m_ScriptClass{nullptr};
        ScriptInstance* m_Instance{nullptr};
    };
}
