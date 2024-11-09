#include "FussionPCH.h"
#include "ScriptAssembly.h"

#include "Fussion/Core/Core.h"
#include "Fussion/Log/Log.h"

#include "scripthelper/scripthelper.h"

#include <ranges>

namespace Fussion {
    ScriptInstance::ScriptInstance(ScriptClass* script_class, asIScriptObject* instance)
        : m_script_class(script_class)
        , m_script_instance(instance)
    {
        (void)m_script_instance->AddRef();
        m_context = m_script_instance->GetEngine()->CreateContext();

        m_context->SetExceptionCallback(asMETHOD(ScriptInstance, on_script_exception), this, asCALL_THISCALL);
    }

    ScriptInstance::~ScriptInstance()
    {
        if (m_context)
            (void)m_context->Release();
        if (m_script_instance)
            (void)m_script_instance->Release();
    }

    ScriptInstance::ScriptInstance(ScriptInstance const& other)
    {
        m_script_instance = other.m_script_instance;
        m_context = other.m_context;
        m_script_class = other.m_script_class;

        (void)m_script_instance->AddRef();
        (void)m_context->AddRef();
    }

    ScriptInstance& ScriptInstance::operator=(ScriptInstance const& other)
    {
        m_script_instance = other.m_script_instance;
        m_context = other.m_context;
        m_script_class = other.m_script_class;

        (void)m_script_instance->AddRef();
        (void)m_context->AddRef();
        return *this;
    }

    void ScriptInstance::call_method(std::string_view name, std::initializer_list<std::any> args)
    {
        if (auto m = m_script_class->method_from_name(std::string(name))) {
            m_context->Prepare(m);
            m_context->SetObject(m_script_instance);
            u32 i = 0;

            for (std::any const& arg : args) {
                if (f32 const* f = std::any_cast<f32>(&arg)) {
                    m_context->SetArgFloat(i, *f);
                } else if (f64 const* d = std::any_cast<f64>(&arg)) {
                    m_context->SetArgDouble(i, *d);
                } else if (u32 const* dw = std::any_cast<u32>(&arg)) {
                    m_context->SetArgDWord(i, *dw);
                } else if (u64 const* qw = std::any_cast<u64>(&arg)) {
                    m_context->SetArgQWord(i, *qw);
                } else {
                    LOG_ERRORF("Unsupported argument type");
                }
                ++i;
            }
            m_context->Execute();
        }
    }

    void ScriptInstance::on_script_exception()
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
        reload(type);
    }

    auto ScriptClass::create_instance() -> ScriptInstance
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

    auto ScriptClass::method_from_name(std::string const& name) -> asIScriptFunction*
    {
        if (m_Methods.contains(name)) {
            return m_Methods[name];
        }
        return nullptr;
    }

    auto ScriptClass::methods() -> std::unordered_map<std::string, asIScriptFunction*>&
    {
        return m_Methods;
    }

    bool ScriptClass::derives_from(std::string const& name) const
    {
        auto type = m_Type->GetModule()->GetTypeInfoByName(name.c_str());
        return m_Type->DerivesFrom(type);
    }

    void ScriptClass::reload(asITypeInfo* type_info)
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
            prop.index = i;

            char const* name;
            m_Type->GetProperty(i, &name, TRANSMUTE(int*, &prop.type_id), &prop.is_private, &prop.is_protected, &prop.offset, &prop.is_reference);

            m_Properties[name] = prop;
        }
    }

    auto ScriptClass::property_from_name(std::string const& name) -> ScriptProperty
    {
        return m_Properties[name];
    }

    ScriptAssembly::ScriptAssembly(asIScriptModule* module)
    {
        reload(module);
    }

    auto ScriptAssembly::klass(std::string const& name) -> Maybe<ScriptClass*>
    {
        if (m_classes.contains(name))
            return &m_classes[name];
        return None();
    }

    auto ScriptAssembly::classes_of_type(std::string const& type) -> std::vector<ScriptClass*>
    {
        std::vector<ScriptClass*> ret;
        for (auto& klass : m_classes | std::views::values) {
            if (klass.type_info()->GetBaseType()->GetName() == type) {
                ret.push_back(&klass);
            }
        }
        return ret;
    }

    void ScriptAssembly::reload(asIScriptModule* module)
    {
        m_module = module;
        m_name = module->GetName();
        for (u32 i = 0; i < module->GetObjectTypeCount(); i++) {
            auto const klass = module->GetObjectTypeByIndex(i);
            m_classes[klass->GetName()] = ScriptClass(klass);
        }
    }
}
