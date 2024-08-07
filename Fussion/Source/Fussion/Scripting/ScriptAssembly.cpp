#include "e5pch.h"
#include "ScriptAssembly.h"

#include "Fussion/Core/Core.h"
#include "Fussion/Log/Log.h"
#include "RHI/FrameBuffer.h"

#include <ranges>

namespace Fussion {

// auto HasAttribute::GetAttribute(std::string_view name) -> std::optional<Ptr<Scripting::Attribute>&>
// {
//     // if (auto attr = std::ranges::find_if(m_Attributes, [name](Ptr<Scripting::Attribute> const& attribute) {
//     //     return name.empty();
//     // }); attr != m_Attributes.end()) {
//     //     return attr;
//     // }
//     return std::nullopt;
// }

ScriptInstance::ScriptInstance(ScriptClass* script_class, asIScriptObject* instance): m_ScriptClass(script_class),
                                                                                      m_Instance(instance)
{
    (void)m_Instance->AddRef();
    m_Context = m_Instance->GetEngine()->CreateContext();
}

ScriptInstance::~ScriptInstance()
{
    if (m_Context)
        (void)m_Context->Release();
    if (m_Instance)
        (void)m_Instance->Release();
}

ScriptInstance::ScriptInstance(ScriptInstance const& other)
{
    m_Instance = other.m_Instance;
    m_Context = other.m_Context;
    m_ScriptClass = other.m_ScriptClass;

    (void)m_Instance->AddRef();
    (void)m_Context->AddRef();
}

ScriptInstance& ScriptInstance::operator=(ScriptInstance const& other)
{
    m_Instance = other.m_Instance;
    m_Context = other.m_Context;
    m_ScriptClass = other.m_ScriptClass;

    (void)m_Instance->AddRef();
    (void)m_Context->AddRef();
    return *this;
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


auto ScriptClass::GetMethods() -> std::unordered_map<std::string, asIScriptFunction*>& {
    return m_Methods;
}

bool ScriptClass::DerivesFrom(std::string const& name) const {
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

        const char* name;
        m_Type->GetProperty(i, &name, TRANSMUTE(int*, &prop.TypeId), &prop.IsPrivate, &prop.IsProtected, &prop.Offset, &prop.IsReference);

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

auto ScriptAssembly::GetClass(std::string const& name) -> std::optional<ScriptClass*>
{
    if (m_Classes.contains(name))
        return &m_Classes[name];
    return std::nullopt;
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
        LOG_DEBUGF("Storing class: {}", klass->GetName());
        m_Classes[klass->GetName()] = ScriptClass(klass);
    }
}
}
