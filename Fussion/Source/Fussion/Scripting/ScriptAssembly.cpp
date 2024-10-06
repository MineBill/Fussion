#include "FussionPCH.h"
#include "ScriptAssembly.h"

#include "Fussion/Core/Core.h"
#include "Fussion/Log/Log.h"

#include "scripthelper/scripthelper.h"

#include <ranges>

namespace Fussion {
    ScriptInstance::ScriptInstance(ScriptClass* script_class, asIScriptObject* instance)
        : m_ScriptClass(script_class)
        , m_Instance(instance)
    {
        (void)m_Instance->AddRef();
        m_context = m_Instance->GetEngine()->CreateContext();

        m_context->SetExceptionCallback(asMETHOD(ScriptInstance, OnScriptException), this, asCALL_THISCALL);
    }

    ScriptInstance::~ScriptInstance()
    {
        if (m_context)
            (void)m_context->Release();
        if (m_Instance)
            (void)m_Instance->Release();
    }

    ScriptInstance::ScriptInstance(ScriptInstance const& other)
    {
        m_Instance = other.m_Instance;
        m_context = other.m_context;
        m_ScriptClass = other.m_ScriptClass;

        (void)m_Instance->AddRef();
        (void)m_context->AddRef();
    }

    ScriptInstance& ScriptInstance::operator=(ScriptInstance const& other)
    {
        m_Instance = other.m_Instance;
        m_context = other.m_context;
        m_ScriptClass = other.m_ScriptClass;

        (void)m_Instance->AddRef();
        (void)m_context->AddRef();
        return *this;
    }

    void ScriptInstance::CallMethod(std::string_view name, std::initializer_list<std::any> args)
    {
        if (auto m = m_ScriptClass->GetMethod(std::string(name))) {
            m_context->Prepare(m);
            m_context->SetObject(m_Instance);
            u32 i = 0;

            for (std::any const& arg : args) {
                if (f32 const* f = std::any_cast<f32>(&arg)) {
                    m_context->SetArgFloat(i, *f);
                } else if (f64 const* f = std::any_cast<f64>(&arg)) {
                    m_context->SetArgDouble(i, *f);
                } else if (u32 const* f = std::any_cast<u32>(&arg)) {
                    m_context->SetArgDWord(i, *f);
                } else if (u64 const* f = std::any_cast<u64>(&arg)) {
                    m_context->SetArgQWord(i, *f);
                } else {
                    LOG_ERRORF("Unsupported argument type");
                }
                ++i;
            }
            m_context->Execute();
        }
    }

    void ScriptInstance::OnScriptException()
    {
        LOG_ERRORF("Script exception occured: {}", m_context->GetExceptionString());

        int column;
        char const* sectionName;
        int lineNumber = m_context->GetExceptionLineNumber(&column, &sectionName);

        auto* function = m_context->GetExceptionFunction();
        LOG_ERROR("Additional info:");
        LOG_ERRORF("\tPosition: {}:{}", lineNumber, column);
        LOG_ERRORF("\tFile: {}", sectionName);
        LOG_ERRORF("\tFunction: {}", function->GetName());

        LOG_ERROR("<< CALLSTACK >>");
        for (asUINT n = 1; n < m_context->GetCallstackSize(); n++) {
            function = m_context->GetFunction(n);
            if (function) {
                if (function->GetFuncType() == asFUNC_SCRIPT) {
                    auto scriptSectionName = (function->GetScriptSectionName() ? function->GetScriptSectionName() : "");
                    LOG_ERRORF("\t{} ({}): {}", scriptSectionName, m_context->GetLineNumber(n), function->GetDeclaration());
                } else {
                    // The context is being reused by the application for a nested call
                    LOG_ERRORF("\t[...Application...]: {}", function->GetDeclaration());
                }
            } else {
                // The context is being reused by the script engine for a nested call
                LOG_ERROR("\t[...Script Engine...]");
            }
        }
    }

    ScriptClass::ScriptClass(asITypeInfo* type)
    {
        Reload(type);
    }

    auto ScriptClass::CreateInstance() -> ScriptInstance
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

    auto ScriptClass::GetMethod(std::string const& name) -> asIScriptFunction*
    {
        if (m_Methods.contains(name)) {
            return m_Methods[name];
        }
        return nullptr;
    }

    auto ScriptClass::GetMethods() -> std::unordered_map<std::string, asIScriptFunction*>&
    {
        return m_Methods;
    }

    bool ScriptClass::DerivesFrom(std::string const& name) const
    {
        auto type = m_Type->GetModule()->GetTypeInfoByName(name.c_str());
        return m_Type->DerivesFrom(type);
    }

    void ScriptClass::Reload(asITypeInfo* type_info)
    {
        m_Type = type_info;
        m_Name = m_Type->GetName();
        auto decl = std::format("{} @{}()", m_Name, m_Name);
        m_Factory = m_Type->GetFactoryByDecl(decl.c_str());
        if (m_Factory == nullptr) {
            LOG_ERRORF("Failed to find factory '{}' for class: {}", decl, m_Name);
        }

        for (u32 i = 0; i < m_Type->GetMethodCount(); i++) {
            auto const method = m_Type->GetMethodByIndex(i);

            m_Methods[method->GetName()] = method;
        }

        for (u32 i = 0; i < m_Type->GetPropertyCount(); i++) {
            ScriptProperty prop;
            prop.Index = i;

            char const* name;
            m_Type->GetProperty(i, &name, TRANSMUTE(int*, &prop.TypeID), &prop.IsPrivate, &prop.IsProtected, &prop.Offset, &prop.IsReference);

            m_Properties[name] = prop;
        }
    }

    auto ScriptClass::GetProperty(std::string const& name) -> ScriptProperty
    {
        return m_Properties[name];
    }

    ScriptAssembly::ScriptAssembly(asIScriptModule* module)
    {
        Reload(module);
    }

    auto ScriptAssembly::GetClass(std::string const& name) -> Maybe<ScriptClass*>
    {
        if (m_Classes.contains(name))
            return &m_Classes[name];
        return None();
    }

    auto ScriptAssembly::GetClassesOfType(std::string const& type) -> std::vector<ScriptClass*>
    {
        std::vector<ScriptClass*> ret;
        for (auto& klass : m_Classes | std::views::values) {
            if (klass.GetTypeInfo()->GetBaseType()->GetName() == type) {
                ret.push_back(&klass);
            }
        }
        return ret;
    }

    void ScriptAssembly::Reload(asIScriptModule* module)
    {
        m_Module = module;
        m_Name = module->GetName();
        for (u32 i = 0; i < module->GetObjectTypeCount(); i++) {
            auto const klass = module->GetObjectTypeByIndex(i);
            m_Classes[klass->GetName()] = ScriptClass(klass);
        }
    }
}
