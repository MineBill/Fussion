#include "e5pch.h"
#include "ScriptComponent.h"

#include "Scripting/ScriptBase.h"

namespace Fussion {
void ScriptComponent::OnStart()
{
    if (auto klass = ScriptingEngine::Get().GetGameAssembly()->GetClass(ClassName); klass) {
        if (klass.value()->DerivesFrom("Script")) {
            m_Instance = klass.value()->CreateInstance();

            if (m_Instance.IsValid()) {
                m_Instance.SetProperty("m_Owner", m_Owner);

                m_Instance.CallMethod("OnStart");
            }
        }
    }
}

void ScriptComponent::OnUpdate(f32 delta)
{
    if (m_Instance.IsValid()) {
        m_Instance.CallMethod("OnUpdate");
    }
}

void ScriptComponent::OnDestroy() {}

void ScriptComponent::Test() const
{
    if (auto klass = ScriptingEngine::Get().GetGameAssembly()->GetClass(ClassName); klass) {
        if (klass.value()->DerivesFrom("Script")) {
            if (auto inst = klass.value()->CreateInstance(); inst.IsValid()) {
                inst.SetProperty("m_Owner", m_Owner);
                inst.CallMethod("OnStart");
            } else {
                LOG_WARNF("Instance not valid");
            }
        } else {
            LOG_DEBUGF("Script does not derive from ScriptBase");
        }
    } else {
        LOG_WARNF("Class '{}' not found", ClassName);
    }
}

}
