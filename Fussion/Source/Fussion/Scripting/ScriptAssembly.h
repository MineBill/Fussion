#pragma once
#include "angelscript.h"
#include "Fussion/Core/Types.h"

namespace Fussion
{
    class ScriptClass;

    class ScriptInstance {
    public:
        ScriptInstance() = default;
        explicit ScriptInstance(ScriptClass* script_class, asIScriptObject* instance);
        ~ScriptInstance();

        bool IsValid() const { return m_Instance != nullptr; }

        asIScriptObject* GetInstance() const { return m_Instance; }
        ScriptClass* GetScriptClass() const { return m_ScriptClass; }

        void CallMethod(std::string const& method);
    private:

        ScriptClass* m_ScriptClass;
        asIScriptObject* m_Instance;
    };

    struct ScriptProperty {
        std::string Name;
        bool IsPrivate;
        bool IsProtected;
        asETypeIdFlags TypeId;

        int Offset;
        bool IsReference;
    };

    class ScriptClass {
    public:
        ScriptClass() = default;
        explicit ScriptClass(asITypeInfo* type);

        std::string const& GetName() { return m_Name; }

        ScriptInstance CreateInstance();
        asIScriptFunction* GetMethod(std::string const& name);

        asITypeInfo* GetTypeInfo() const { return m_Type; }
        std::unordered_map<std::string, ScriptProperty> GetProperties() const { return m_Properties; }

    private:
        std::unordered_map<std::string, asIScriptFunction*> m_Methods;
        std::unordered_map<std::string, ScriptProperty> m_Properties;

        std::string m_Name;
        asIScriptFunction* m_Factory;
        asITypeInfo *m_Type;
    };

    class ScriptAssembly
    {
        ScriptAssembly();
    public:
        explicit ScriptAssembly(asIScriptModule* module);

        std::optional<ScriptClass> GetClass(std::string const& name)
        {
            if (m_Classes.contains(name))
                return m_Classes[name];
            return std::nullopt;
        }

        std::unordered_map<std::string, ScriptClass>& GetAllClasses() { return m_Classes; }
        std::vector<ScriptClass*> GetClassesOfType(std::string const& type);

    private:
        std::unordered_map<std::string, ScriptClass> m_Classes;
        asIScriptModule* m_Module{nullptr};
        std::string m_Name;
    };
}
