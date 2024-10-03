#pragma once
#include <Fussion/Scripting/Attribute.h>
#include <Fussion/Scripting/ScriptAssembly.h>

#include "angelscript.h"

#include <sstream>

namespace Fussion {
    class ScriptingEngine {
        ScriptingEngine();

    public:
        static ScriptingEngine& Self() { return *s_Instance; }

        static void Initialize();
        static void Shutdown();

        auto DumpCurrentTypes() const -> std::stringstream;

        void RegisterTypes();

        /**
         * Reads all scripts in path and compiles them into a module. If the module
         * already exists, it is reloaded, otherwise a new module is created.
         * @param path Path to the assembly folder.
         * @param module_name The name of the assembly.
         * @return
         */
        auto CompileAssembly(std::filesystem::path const& path, std::string const& module_name) -> Ref<ScriptAssembly>;

        void ParseAttributes(CScriptBuilder& builder, ScriptAssembly& assembly);

        auto CompileGameAssembly(std::filesystem::path const& path) -> Ref<ScriptAssembly>;
        auto GetGameAssembly() -> Ref<ScriptAssembly>;

        auto GetAssembly(std::string const& name) -> Ref<ScriptAssembly> { return m_LoadedAssemblies[name]; }
        auto GetLoadedAssemblies() -> std::unordered_map<std::string, Ref<ScriptAssembly>>& { return m_LoadedAssemblies; }

        template<typename T>
        static auto GetAttribute(Uuid uuid) -> T*
        {
            for (auto& attribute : s_Instance->m_Attributes[uuid]) {
                if (auto attr = dynamic_cast<T*>(attribute.get())) {
                    return attr;
                }
            }
            return nullptr;
        }

    private:
        static ScriptingEngine* s_Instance;

        std::unordered_map<Uuid, std::vector<Ptr<Scripting::Attribute>>> m_Attributes;
        std::unordered_map<std::string, Ref<ScriptAssembly>> m_LoadedAssemblies;
        asIScriptEngine* m_ScriptEngine;
    };
}
