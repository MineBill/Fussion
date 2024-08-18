#include "FussionPCH.h"
#include "ScriptBase.h"

namespace Fussion {
ScriptBase::ScriptBase(Entity* owner): m_Owner(owner) {}

ScriptBase::ScriptBase(Entity* owner, asIScriptObject* object): m_Owner(owner), m_ScriptObject(object) {
}

ScriptBase* ScriptBase::Create(Entity* owner)
{
    asIScriptContext* ctx = asGetActiveContext();

    // Get the function that is calling the factory so we can be certain it is the FooScript script class
    asIScriptFunction* func = ctx->GetFunction(0);
    if (func->GetObjectType() == nullptr || std::string(func->GetObjectType()->GetName()) != "ScriptBase") {
        ctx->SetException("Invalid attempt to manually instantiate FooScript_t");
        return nullptr;
    }

    // Get the this pointer from the calling function so the FooScript C++
    // class can be linked with the FooScript script class
    auto obj = static_cast<asIScriptObject*>(ctx->GetThisPointer(0));

    return new ScriptBase(owner, obj);
}

ScriptBase& ScriptBase::operator=(ScriptBase const& s)
{
    m_Owner = s.m_Owner;
    return *this;
}

void ScriptBase::OnStart()
{
    LOG_DEBUGF("ScriptBase::OnStart: Shit mista");
    auto engine = m_ScriptObject->GetEngine();
    auto active = asGetActiveContext();
    if (active != nullptr) {
        auto func = active->GetFunction(0);
        if (strcmp(func->GetName(), "OnStart") != 0 || active->GetThisPointer(0) != m_ScriptObject) {
            auto ctx = engine->RequestContext();

            ctx->Prepare(m_ScriptObject->GetObjectType()->GetMethodByDecl("void OnStart()"));
            ctx->SetObject(m_ScriptObject);
            ctx->Execute();
            engine->ReturnContext(ctx);
        }
    }
}

void ScriptBase::OnUpdate(f32 delta) {}

Entity* ScriptBase::GetOwner() const
{
    return m_Owner;
}

void ScriptBase::AddRef()
{
    m_RefCount++;
}

void ScriptBase::Release()
{
    m_RefCount--;
    if (m_RefCount == 0) {
        LOG_WARNF("RefCount is 0");
    }
}

}
