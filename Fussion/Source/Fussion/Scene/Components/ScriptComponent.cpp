#include "FussionPCH.h"
#include "ScriptComponent.h"
#include "Scripting/ScriptBase.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void ScriptComponent::OnStart()
    {
        if (auto klass = ScriptingEngine::Get().GetGameAssembly()->GetClass(ClassName); klass) {
            if (klass.Value()->DerivesFrom("Script")) {
                m_Instance = klass.Value()->CreateInstance();

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
            m_Instance.CallMethod("OnUpdate", delta);
        }
    }

    void ScriptComponent::OnDestroy() {}

    void ScriptComponent::Test() const
    {
        if (auto klass = ScriptingEngine::Get().GetGameAssembly()->GetClass(ClassName); klass) {
            if (klass.Value()->DerivesFrom("Script")) {
                if (auto inst = klass.Value()->CreateInstance(); inst.IsValid()) {
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

    Ref<Component> ScriptComponent::Clone()
    {
        auto script = MakeRef<ScriptComponent>();
        script->ClassName = ClassName;
        return script;
    }

    void ScriptComponent::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(ClassName);
    }

    void ScriptComponent::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(ClassName);
    }
}
