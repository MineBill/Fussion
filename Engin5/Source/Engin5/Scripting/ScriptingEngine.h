#pragma once
#include "angelscript.h"
#include "ScriptAssembly.h"

#include <sstream>

namespace Engin5
{
    class ScriptingEngine
    {
        ScriptingEngine();
    public:
        static ScriptingEngine& Get() { return *s_Instance; }

        static void Initialize();
        static void Shutdown();

        void LoadScriptFromPath(std::filesystem::path path);

        std::stringstream DumpCurrentTypes() const;

        Ref<ScriptAssembly> CompileAssembly(std::filesystem::path path, std::string const& module_name);

        Ref<ScriptAssembly> GetAssembly(std::string const& name) { return m_LoadedAssemblies[name]; }
        std::unordered_map<std::string, Ref<ScriptAssembly>>& GetLoadedAssemblies() { return m_LoadedAssemblies; }

    private:
        static ScriptingEngine* s_Instance;

        std::unordered_map<std::string, Ref<ScriptAssembly>> m_LoadedAssemblies;
        asIScriptEngine* m_ScriptEngine;
        asIScriptModule* m_GameModule;
    };
}
