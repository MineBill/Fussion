#include "e5pch.h"
#include "ScriptAssembly.h"

#include "Fussion/Core/Core.h"
#include "Fussion/Log/Log.h"
#include "RHI/FrameBuffer.h"

namespace Fussion
{
    ScriptInstance::ScriptInstance(ScriptClass* script_class, asIScriptObject* instance): m_ScriptClass(script_class),
        m_Instance(instance)
    {
        m_Instance->AddRef();
    }

    ScriptInstance::~ScriptInstance()
    {
        m_Instance->Release();
    }

    void ScriptInstance::CallMethod(std::string const& method)
    {
        auto  m = m_ScriptClass->GetMethod(method);
        if (m) {
            auto const ctx = m_Instance->GetEngine()->CreateContext();
            ctx->Prepare(m);
            ctx->SetObject(m_Instance);
            ctx->Execute();
        }
    }

    ScriptClass::ScriptClass(asITypeInfo* type): m_Type(type)
    {
        m_Name = type->GetName();
        auto decl = std::format("{} @{}()", type->GetName(), type->GetName());
        m_Factory = type->GetFactoryByDecl(decl.c_str());
        if (m_Factory == nullptr) {
            LOG_ERRORF("Failed to find factory '{}' for class: {}", decl, type->GetName());
        }

        for (u32 i = 0; i < type->GetMethodCount(); i++) {
            auto const method = type->GetMethodByIndex(i);

            m_Methods[method->GetName()] = method;
        }

        for (u32 i = 0; i < type->GetPropertyCount(); i++) {
            ScriptProperty prop;

            const char* name;
            type->GetProperty(i, &name, TRANSMUTE(int*, &prop.TypeId), &prop.IsPrivate, &prop.IsProtected, &prop.Offset, &prop.IsReference);

            m_Properties[name] = prop;
        }
    }

    ScriptInstance ScriptClass::CreateInstance()
    {
        auto ctx = m_Type->GetEngine()->CreateContext();
        defer(ctx->Release());
        ctx->Prepare(m_Factory);

        auto status = ctx->Execute();
        if (status == asEXECUTION_FINISHED) {
            asIScriptObject* obj = *static_cast<asIScriptObject**>(ctx->GetAddressOfReturnValue());
            return ScriptInstance(this, obj);
        }

        return {};
    }

    asIScriptFunction* ScriptClass::GetMethod(std::string const& name)
    {
        if (m_Methods.contains(name)) {
            return m_Methods[name];
        }
        return nullptr;
    }

    ScriptAssembly::ScriptAssembly()
    {
    }

    ScriptAssembly::ScriptAssembly(asIScriptModule* module): m_Module(module)
    {
        m_Name = module->GetName();
        for (u32 i = 0; i < module->GetObjectTypeCount(); i++) {
            auto const klass = module->GetObjectTypeByIndex(i);
            LOG_DEBUGF("Storing class: {}", klass->GetName());
            m_Classes[klass->GetName()] = ScriptClass(klass);
        }
    }

    std::vector<ScriptClass*> ScriptAssembly::GetClassesOfType(std::string const& type)
    {
        std::vector<ScriptClass*> ret;
        for (auto& [name, klass] : m_Classes) {
            if (klass.GetTypeInfo()->GetBaseType()->GetName() == type) {
                ret.push_back(&klass);
            }
        }
        return ret;
    }
}