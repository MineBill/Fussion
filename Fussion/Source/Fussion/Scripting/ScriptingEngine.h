#pragma once
#include "angelscript.h"
#include "ScriptAssembly.h"

#include <sstream>

namespace Fussion {
class ScriptingEngine {
    ScriptingEngine();

public:
    static ScriptingEngine& inst() { return *s_instance; }

    static void initialize();
    static void shutdown();

    auto dump_current_types() const -> std::stringstream;

    void register_types();

    /**
     * Reads all scripts in path and compiles them into a module. If the module
     * already exists, it is reloaded, otherwise a new module is created.
     * @param path Path to the assembly folder.
     * @param module_name The name of the assembly.
     * @return
     */
    auto compile_assembly(std::filesystem::path const& path, std::string const& module_name) -> Ref<ScriptAssembly>;

    void parse_attributes(CScriptBuilder& builder, ScriptAssembly& assembly);

    auto compile_game_assembly(std::filesystem::path const& path) -> Ref<ScriptAssembly>;
    auto get_game_assembly() -> Ref<ScriptAssembly>;

    auto get_assembly(std::string const& name) -> Ref<ScriptAssembly> { return m_loaded_assemblies[name]; }
    auto get_loaded_assemblies() -> std::unordered_map<std::string, Ref<ScriptAssembly>>& { return m_loaded_assemblies; }

    template<typename T>
    static auto get_attribute(Uuid uuid) -> T*
    {
        for (auto& attribute: s_instance->m_attributes[uuid]) {
            if (auto attr = dynamic_cast<T*>(attribute.get())) {
                return attr;
            }
        }
        return nullptr;
    }

private:
    static ScriptingEngine* s_instance;

    std::unordered_map<Uuid, std::vector<Ptr<Scripting::Attribute>>> m_attributes;
    std::unordered_map<std::string, Ref<ScriptAssembly>> m_loaded_assemblies;
    asIScriptEngine* m_script_engine;
};
}
