#pragma once
#include <Fussion/Core/Core.h>
#include <Fussion/Core/Maybe.h>
#include <Fussion/Core/Mem.h>
#include <Fussion/Core/Types.h>
#include <Fussion/Core/Uuid.h>

#include <angelscript.h>
#include <scriptbuilder/scriptbuilder.h>

#include <any>

namespace Fussion {
    class ScriptClass;
    class ScriptingEngine;

    class HasAttribute { };

    class ScriptInstance final {
    public:
        ScriptInstance() = default;
        explicit ScriptInstance(ScriptClass* script_class, asIScriptObject* instance);
        ~ScriptInstance();

        ScriptInstance(ScriptInstance const& other);
        ScriptInstance& operator=(ScriptInstance const& other);

        bool is_valid() const { return m_script_instance != nullptr; }

        asIScriptObject* instance() const { return m_script_instance; }
        ScriptClass* script_class() const { return m_script_class; }

        void call_method(std::string_view name, std::initializer_list<std::any> args);

        template<typename T>
        void set_property(std::string const& name, T& value);

        template<typename T>
        T* as()
        {
            return TRANSMUTE(T*, m_script_instance->GetAddressOfProperty(0));
        }

    private:
        void on_script_exception();

        ScriptClass* m_script_class { nullptr };
        asIScriptObject* m_script_instance { nullptr };

        asIScriptContext* m_context { nullptr };
    };

    struct ScriptProperty : HasAttribute {
        u32 index {};
        std::string name {};
        bool is_private {};
        bool is_protected {};
        asETypeIdFlags type_id {};

        int offset {};
        bool is_reference {};

        Uuid id {};
    };

    class ScriptClass : public HasAttribute {
    public:
        ScriptClass() = default;
        explicit ScriptClass(asITypeInfo* type);

        auto name() -> std::string const& { return m_Name; }

        auto create_instance() -> ScriptInstance;

        auto create_instance_with(auto&& func) -> ScriptInstance
        {
            auto ctx = m_Type->GetEngine()->CreateContext();
            defer(ctx->Release());
            ctx->Prepare(m_Factory);

            func(ctx);

            auto status = ctx->Execute();
            if (status == asEXECUTION_FINISHED) {
                asIScriptObject* obj = *static_cast<asIScriptObject**>(ctx->GetAddressOfReturnValue());
                return ScriptInstance(this, obj);
            }

            return {};
        }

        auto method_from_name(std::string const& name) -> asIScriptFunction*;

        [[nodiscard]]
        auto type_info() const -> asITypeInfo*
        {
            return m_Type;
        }

        [[nodiscard]]
        auto properties() const -> std::unordered_map<std::string, ScriptProperty>
        {
            return m_Properties;
        }

        [[nodiscard]]
        auto methods() -> std::unordered_map<std::string, asIScriptFunction*>&;

        bool derives_from(std::string const& name) const;

        void reload(asITypeInfo* type_info);
        auto property_from_name(std::string const& name) -> ScriptProperty;

    private:
        std::unordered_map<std::string, asIScriptFunction*> m_Methods {};
        std::unordered_map<std::string, ScriptProperty> m_Properties {};

        std::string m_Name { "Invalid" };
        asIScriptFunction* m_Factory { nullptr };
        asITypeInfo* m_Type { nullptr };
        Uuid m_ID {};

        friend ScriptingEngine;
    };

    template<typename T>
    void ScriptInstance::set_property(std::string const& name, T& value)
    {
        auto prop = m_script_class->property_from_name(name);
        auto type_name = m_script_instance->GetObjectType()->GetName();
        LOG_DEBUGF("NAME: {}", type_name);
        auto ptr = m_script_instance->GetAddressOfProperty(prop.index);

        Mem::copy(ptr, &value, sizeof(T));
    }

    class ScriptAssembly {
        ScriptAssembly() = default;

    public:
        explicit ScriptAssembly(asIScriptModule* module);

        auto klass(std::string const& name) -> Maybe<ScriptClass*>;

        auto all_classes() -> std::unordered_map<std::string, ScriptClass>& { return m_classes; }
        auto classes_of_type(std::string const& type) -> std::vector<ScriptClass*>;
        void reload(asIScriptModule* module);

    private:
        std::unordered_map<std::string, ScriptClass> m_classes {};
        asIScriptModule* m_module { nullptr };
        std::string m_name {};

        friend ScriptingEngine;
    };
}
