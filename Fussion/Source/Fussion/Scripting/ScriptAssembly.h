#pragma once
#include "Attribute.h"
#include "angelscript.h"
#include "Fussion/Core/Uuid.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Types.h"
#include <Fussion/Core/Maybe.h>
#include "scriptbuilder/scriptbuilder.h"

#include <cstring>

namespace Fussion {
    class ScriptClass;
    class ScriptingEngine;

    class HasAttribute {
        //     auto GetAttribute(std::string_view name) -> std::optional<Ptr<Scripting::Attribute>&>;
        //
        // protected:
        //     std::vector<Ptr<Scripting::Attribute>> m_Attributes{};
        //
        //     friend ScriptingEngine;
    };

    class ScriptInstance final {
    public:
        ScriptInstance() = default;
        explicit ScriptInstance(ScriptClass* script_class, asIScriptObject* instance);
        ~ScriptInstance();

        ScriptInstance(ScriptInstance const& other);
        ScriptInstance& operator=(ScriptInstance const& other);

        bool is_valid() const { return m_instance != nullptr; }

        asIScriptObject* instance() const { return m_instance; }
        ScriptClass* script_class() const { return m_script_class; }

        template<typename... Args>
        void call_method(std::string_view name, Args&&... args);

        template<typename T>
        void set_property(std::string const& name, T& value);

        template<typename T>
        T* as()
        {
            return TRANSMUTE(T*, m_instance->GetAddressOfProperty(0));
        }

    private:
        ScriptClass* m_script_class{ nullptr };
        asIScriptObject* m_instance{ nullptr };

        asIScriptContext* m_context{ nullptr };
    };

    struct ScriptProperty : HasAttribute {
        u32 index{};
        std::string name{};
        bool is_private{};
        bool is_protected{};
        asETypeIdFlags type_id{};

        int offset{};
        bool is_reference{};

        Uuid uuid{};
    };

    class ScriptClass : public HasAttribute {
    public:
        ScriptClass() = default;
        explicit ScriptClass(asITypeInfo* type);

        auto name() -> std::string const& { return m_name; }

        auto create_instance() -> ScriptInstance;

        auto create_instance_with(auto&& func) -> ScriptInstance
        {
            auto ctx = m_type->GetEngine()->CreateContext();
            defer(ctx->Release());
            ctx->Prepare(m_factory);

            func(ctx);

            auto status = ctx->Execute();
            if (status == asEXECUTION_FINISHED) {
                asIScriptObject* obj = *static_cast<asIScriptObject**>(ctx->GetAddressOfReturnValue());
                return ScriptInstance(this, obj);
            }

            return {};
        }

        auto get_method(std::string const& name) -> asIScriptFunction*;

        [[nodiscard]]
        auto get_type_info() const -> asITypeInfo* { return m_type; }

        [[nodiscard]]
        auto get_properties() const -> std::unordered_map<std::string, ScriptProperty> { return m_properties; }

        [[nodiscard]]
        auto get_methods() -> std::unordered_map<std::string, asIScriptFunction*>&;

        bool derives_from(std::string const& name) const;

        void reload(asITypeInfo* type_info);
        auto get_property(std::string const& name) -> ScriptProperty;

    private:
        std::unordered_map<std::string, asIScriptFunction*> m_methods{};
        std::unordered_map<std::string, ScriptProperty> m_properties{};

        std::string m_name{ "Invalid" };
        asIScriptFunction* m_factory{ nullptr };
        asITypeInfo* m_type{ nullptr };
        Uuid m_uuid{};

        friend ScriptingEngine;
    };

    template<typename... Args>
    void ScriptInstance::call_method(std::string_view name, Args&&... args)
    {
        if (auto m = m_script_class->get_method(std::string(name))) {
            m_context->Prepare(m);
            m_context->SetObject(m_instance);
            u32 i = 0;
            ([&]<typename Arg>() {
                using BaseType = std::remove_cvref_t<Arg>;

                if constexpr (std::is_same_v<BaseType, f32>) {
                    m_context->SetArgFloat(i, args);
                } else if constexpr (std::is_same_v<BaseType, f64>) {
                    m_context->SetArgDouble(i, args);
                } else if constexpr (std::is_same_v<BaseType, u32>) {
                    m_context->SetArgDWord(i, args);
                } else if constexpr (std::is_same_v<BaseType, u64>) {
                    m_context->SetArgQWord(i, args);
                } else if constexpr (std::is_same_v<BaseType, std::string>) {
                    m_context->SetArgObject(i, &args);
                } else {
                    static_assert(false, "Unsupported arg type");
                }

                i++;
            }.template operator()<Args>(), ...);
            m_context->Execute();
        }
    }

    template<typename T>
    void ScriptInstance::set_property(std::string const& name, T& value)
    {
        auto prop = m_script_class->get_property(name);
        auto type_name = m_instance->GetObjectType()->GetName();
        LOG_DEBUGF("NAME: {}", type_name);
        auto ptr = m_instance->GetAddressOfProperty(prop.index);

        std::memcpy(ptr, &value, sizeof(T));
    }

    class ScriptAssembly {
        ScriptAssembly() = default;

    public:
        explicit ScriptAssembly(asIScriptModule* module);

        auto get_class(std::string const& name) -> Maybe<ScriptClass*>;

        auto get_all_classes() -> std::unordered_map<std::string, ScriptClass>& { return m_classes; }
        auto get_classes_of_type(std::string const& type) -> std::vector<ScriptClass*>;
        void reload(asIScriptModule* module);

    private:
        std::unordered_map<std::string, ScriptClass> m_classes{};
        asIScriptModule* m_module{ nullptr };
        std::string m_name{};

        friend ScriptingEngine;
    };
}
