#pragma once
#include "Fussion/Scene/Component.h"
#include "Fussion/Scripting/ScriptAssembly.h"
#include "Fussion/Scripting/ScriptingEngine.h"

namespace Fussion {
class ScriptComponent final : public Component {
public:
    COMPONENT_DEFAULT(ScriptComponent)

    virtual void OnStart() override;
    virtual void OnUpdate(f32) override;

    virtual void OnDestroy() override;

    void Test() const;

    std::string ClassName{};

private:
    ScriptClass* m_ScriptClass{ nullptr };
    ScriptInstance m_Instance{};
};
}
