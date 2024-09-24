#include "FussionPCH.h"
#include "ScriptComponent.h"
#include "Scripting/ScriptBase.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void ScriptComponent::on_start()
    {
        if (auto klass = ScriptingEngine::inst().get_game_assembly()->get_class(class_name); klass) {
            if (klass.value()->derives_from("Script")) {
                m_instance = klass.value()->create_instance();

                if (m_instance.is_valid()) {
                    m_instance.set_property("m_Owner", m_owner);

                    m_instance.call_method("OnStart");
                }
            }
        }
    }

    void ScriptComponent::on_update(f32 delta)
    {
        if (m_instance.is_valid()) {
            m_instance.call_method("OnUpdate", { delta });
        }
    }

    void ScriptComponent::on_destroy() {}

    void ScriptComponent::test() const
    {
        if (auto klass = ScriptingEngine::inst().get_game_assembly()->get_class(class_name); klass) {
            if (klass.value()->derives_from("Script")) {
                if (auto inst = klass.value()->create_instance(); inst.is_valid()) {
                    inst.set_property("m_Owner", m_owner);
                    inst.call_method("OnStart");
                } else {
                    LOG_WARNF("Instance not valid");
                }
            } else {
                LOG_DEBUGF("Script does not derive from ScriptBase");
            }
        } else {
            LOG_WARNF("Class '{}' not found", class_name);
        }
    }

    Ref<Component> ScriptComponent::clone()
    {
        auto script = make_ref<ScriptComponent>();
        script->class_name = class_name;
        return script;
    }

    void ScriptComponent::serialize(Serializer& ctx) const
    {
        Component::serialize(ctx);
        FSN_SERIALIZE_MEMBER(class_name);
    }

    void ScriptComponent::deserialize(Deserializer& ctx)
    {
        Component::deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(class_name);
    }
}
