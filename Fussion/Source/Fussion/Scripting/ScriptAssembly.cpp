#include "FussionPCH.h"
#include "ScriptAssembly.h"

#include "Fussion/Core/Core.h"
#include "Fussion/Log/Log.h"

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

ScriptInstance::ScriptInstance(ScriptClass* script_class, asIScriptObject* instance): m_script_class(script_class),
                                                                                      m_instance(instance)
{
    (void)m_instance->AddRef();
    m_context = m_instance->GetEngine()->CreateContext();
}

ScriptInstance::~ScriptInstance()
{
    if (m_context)
        (void)m_context->Release();
    if (m_instance)
        (void)m_instance->Release();
}

ScriptInstance::ScriptInstance(ScriptInstance const& other)
{
    m_instance = other.m_instance;
    m_context = other.m_context;
    m_script_class = other.m_script_class;

    (void)m_instance->AddRef();
    (void)m_context->AddRef();
}

ScriptInstance& ScriptInstance::operator=(ScriptInstance const& other)
{
    m_instance = other.m_instance;
    m_context = other.m_context;
    m_script_class = other.m_script_class;

    (void)m_instance->AddRef();
    (void)m_context->AddRef();
    return *this;
}

ScriptClass::ScriptClass(asITypeInfo* type)
{
    reload(type);
}

auto ScriptClass::create_instance() -> ScriptInstance
{
    auto ctx = m_type->GetEngine()->CreateContext();
    defer(ctx->Release());
    ctx->Prepare(m_factory);

    auto status = ctx->Execute();
    if (status == asEXECUTION_FINISHED) {
        asIScriptObject* obj = *static_cast<asIScriptObject**>(ctx->GetAddressOfReturnValue());
        return ScriptInstance(this, obj);
    }

    return {};
}

auto ScriptClass::get_method(std::string const& name) -> asIScriptFunction*
{
    if (m_methods.contains(name)) {
        return m_methods[name];
    }
    return nullptr;
}


auto ScriptClass::get_methods() -> std::unordered_map<std::string, asIScriptFunction*>& {
    return m_methods;
}

bool ScriptClass::derives_from(std::string const& name) const {
    auto type = m_type->GetModule()->GetTypeInfoByName(name.c_str());
    return m_type->DerivesFrom(type);
}

void ScriptClass::reload(asITypeInfo* type_info)
{
    m_type = type_info;
    m_name = m_type->GetName();
    auto decl = std::format("{} @{}()", m_name, m_name);
    m_factory = m_type->GetFactoryByDecl(decl.c_str());
    if (m_factory == nullptr) {
        LOG_ERRORF("Failed to find factory '{}' for class: {}", decl, m_name);
    }

    for (u32 i = 0; i < m_type->GetMethodCount(); i++) {
        auto const method = m_type->GetMethodByIndex(i);

        m_methods[method->GetName()] = method;
    }

    for (u32 i = 0; i < m_type->GetPropertyCount(); i++) {
        ScriptProperty prop;
        prop.index = i;

        const char* name;
        m_type->GetProperty(i, &name, TRANSMUTE(int*, &prop.type_id), &prop.is_private, &prop.is_protected, &prop.offset, &prop.is_reference);

        m_properties[name] = prop;
    }
}

auto ScriptClass::get_property(std::string const& name) -> ScriptProperty
{
    return m_properties[name];
}

ScriptAssembly::ScriptAssembly(asIScriptModule* module)
{
    reload(module);
}

auto ScriptAssembly::get_class(std::string const& name) -> Maybe<ScriptClass*>
{
    if (m_classes.contains(name))
        return &m_classes[name];
    return None();
}

auto ScriptAssembly::get_classes_of_type(std::string const& type) -> std::vector<ScriptClass*>
{
    std::vector<ScriptClass*> ret;
    for (auto& klass : m_classes | std::views::values) {
        if (klass.get_type_info()->GetBaseType()->GetName() == type) {
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
