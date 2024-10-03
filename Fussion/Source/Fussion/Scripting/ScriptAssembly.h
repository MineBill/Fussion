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

        bool IsValid() const { return m_Instance != nullptr; }

        asIScriptObject* Instance() const { return m_Instance; }
        ScriptClass* GetScriptClass() const { return m_ScriptClass; }

        template<typename... Args>
        void CallMethod(std::string_view name, Args&&... args);

        void CallMethod(std::string_view name, std::initializer_list<std::any> args);

        template<typename T>
        void SetProperty(std::string const& name, T& value);

        template<typename T>
        T* As()
        {
            return TRANSMUTE(T*, m_Instance->GetAddressOfProperty(0));
        }

    private:
        ScriptClass* m_ScriptClass { nullptr };
        asIScriptObject* m_Instance { nullptr };

        asIScriptContext* m_context { nullptr };
    };

    struct ScriptProperty : HasAttribute {
        u32 Index {};
        std::string Name {};
        bool IsPrivate {};
        bool IsProtected {};
        asETypeIdFlags TypeID {};

        int Offset {};
        bool IsReference {};

        Uuid ID {};
    };

    class ScriptClass : public HasAttribute {
    public:
        ScriptClass() = default;
        explicit ScriptClass(asITypeInfo* type);

        auto Name() -> std::string const& { return m_Name; }

        auto CreateInstance() -> ScriptInstance;

        auto CreateInstanceWith(auto&& func) -> ScriptInstance
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

        auto GetMethod(std::string const& name) -> asIScriptFunction*;

        [[nodiscard]]
        auto GetTypeInfo() const -> asITypeInfo*
        {
            return m_Type;
        }

        [[nodiscard]]
        auto GetProperties() const -> std::unordered_map<std::string, ScriptProperty>
        {
            return m_Properties;
        }

        [[nodiscard]]
        auto GetMethods() -> std::unordered_map<std::string, asIScriptFunction*>&;

        bool DerivesFrom(std::string const& name) const;

        void Reload(asITypeInfo* type_info);
        auto GetProperty(std::string const& name) -> ScriptProperty;

    private:
        std::unordered_map<std::string, asIScriptFunction*> m_Methods {};
        std::unordered_map<std::string, ScriptProperty> m_Properties {};

        std::string m_Name { "Invalid" };
        asIScriptFunction* m_Factory { nullptr };
        asITypeInfo* m_Type { nullptr };
        Uuid m_ID {};

        friend ScriptingEngine;
    };

    template<typename... Args>
    void ScriptInstance::CallMethod(std::string_view name, Args&&... args)
    {
        if (auto m = m_ScriptClass->GetMethod(std::string(name))) {
            m_context->Prepare(m);
            m_context->SetObject(m_Instance);
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
            }.template operator()<Args>(),
                ...);
            m_context->Execute();
        }
    }

    template<typename T>
    void ScriptInstance::SetProperty(std::string const& name, T& value)
    {
        auto prop = m_ScriptClass->GetProperty(name);
        auto type_name = m_Instance->GetObjectType()->GetName();
        LOG_DEBUGF("NAME: {}", type_name);
        auto ptr = m_Instance->GetAddressOfProperty(prop.Index);

        Mem::Copy(ptr, &value, sizeof(T));
    }

    class ScriptAssembly {
        ScriptAssembly() = default;

    public:
        explicit ScriptAssembly(asIScriptModule* module);

        auto GetClass(std::string const& name) -> Maybe<ScriptClass*>;

        auto GetAllClasses() -> std::unordered_map<std::string, ScriptClass>& { return m_Classes; }
        auto GetClassesOfType(std::string const& type) -> std::vector<ScriptClass*>;
        void Reload(asIScriptModule* module);

    private:
        std::unordered_map<std::string, ScriptClass> m_Classes {};
        asIScriptModule* m_Module { nullptr };
        std::string m_Name {};

        friend ScriptingEngine;
    };
}
