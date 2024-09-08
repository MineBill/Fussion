#include "FussionPCH.h"
#include "ScriptBase.h"

namespace Fussion {
    ScriptBase::ScriptBase(Entity* owner): m_owner(owner) {}

    ScriptBase::ScriptBase(Entity* owner, asIScriptObject* object): m_owner(owner), m_script_object(object) {}

    ScriptBase* ScriptBase::create(Entity* owner)
    {
        asIScriptContext* ctx = asGetActiveContext();

        // Get the function that is calling the factory, so we can be certain it is the FooScript script class
        asIScriptFunction* func = ctx->GetFunction(0);
        if (func->GetObjectType() == nullptr || std::string(func->GetObjectType()->GetName()) != "ScriptBase") {
            ctx->SetException("Invalid attempt to manually instantiate FooScript_t");
            return nullptr;
        }

        // Get the 'this' pointer from the calling function so the FooScript C++
        // class can be linked with the FooScript script class
        auto obj = static_cast<asIScriptObject*>(ctx->GetThisPointer(0));

        return new ScriptBase(owner, obj);
    }

    ScriptBase& ScriptBase::operator=(ScriptBase const& s)
    {
        m_owner = s.m_owner;
        m_script_object = s.m_script_object;
        return *this;
    }

    void ScriptBase::on_start() const
    {
        auto engine = m_script_object->GetEngine();
        auto active = asGetActiveContext();
        if (active != nullptr) {
            auto func = active->GetFunction(0);
            if (strcmp(func->GetName(), "OnStart") != 0 || active->GetThisPointer(0) != m_script_object) {
                auto ctx = engine->RequestContext();

                ctx->Prepare(m_script_object->GetObjectType()->GetMethodByDecl("void OnStart()"));
                ctx->SetObject(m_script_object);
                ctx->Execute();
                engine->ReturnContext(ctx);
            }
        }
    }

    void ScriptBase::on_update([[maybe_unused]] f32 delta) {}

    Entity* ScriptBase::get_owner() const
    {
        return m_owner;
    }

    void ScriptBase::add_ref()
    {
        m_ref_count++;
    }

    void ScriptBase::release()
    {
        m_ref_count--;
        if (m_ref_count == 0) {
            LOG_WARNF("RefCount is 0");
        }
    }
}
