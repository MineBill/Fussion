#pragma once
#include "Engin5/Scene/Component.h"
#include "Scripting/ScriptAssembly.h"
#include "Scripting/ScriptingEngine.h"
#include "Generated/ScriptComponent_reflect_generated.h"

namespace Engin5
{
    REFLECT_CLASS()
    class ScriptComponent: public Component
    {
        REFLECT_GENERATED_BODY()
    public:
        ScriptComponent() = default;
        COMPONENT(ScriptComponent)

        void OnUpdate(f32) override;

    private:
        ScriptClass* m_ScriptClass{nullptr};
        ScriptInstance* m_Instance{nullptr};
    };
}
