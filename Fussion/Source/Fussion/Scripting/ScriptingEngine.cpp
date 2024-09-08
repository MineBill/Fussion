#include "FussionPCH.h"
#include "Fussion/Scripting/ScriptingEngine.h"
#include "AttributeParser.h"
#include "Util/SimpleLexer.h"
#include "Fussion/Scripting/AngelDumper.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Log/Log.h"

#include "scriptbuilder/scriptbuilder.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

namespace Fussion {
ScriptingEngine* ScriptingEngine::s_instance = nullptr;

void MessageCallback(asSMessageInfo const* msg, [[maybe_unused]] void* param)
{
    if (msg->type == asMSGTYPE_ERROR)
        LOG_ERRORF("{}({}, {}): {}", msg->section, msg->row, msg->col, msg->message);
    else if (msg->type == asMSGTYPE_WARNING)
        LOG_WARNF("{}({}, {}): {}", msg->section, msg->row, msg->col, msg->message);
    else if (msg->type == asMSGTYPE_INFORMATION)
        LOG_INFOF("{}({}, {}): {}", msg->section, msg->row, msg->col, msg->message);
}

ScriptingEngine::ScriptingEngine()
{
    asSetGlobalMemoryFunctions([](size_t size) -> void* {
            auto ptr = malloc(size);
            TracyAllocN(ptr, size, "AngelScript");
            return ptr;
        }, [](void* ptr) {
            TracyFreeN(ptr, "AngelScript");
            free(ptr);
        });
    m_script_engine = asCreateScriptEngine();
    m_script_engine->SetUserData(this);

    auto r = m_script_engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);
    VERIFY(r >= 0, "Failed to set message callback");

    register_types();
}

void ScriptingEngine::initialize()
{
    s_instance = new ScriptingEngine;
}

void ScriptingEngine::shutdown()
{
    delete s_instance;
    s_instance = nullptr;
}

auto ScriptingEngine::dump_current_types() const -> std::stringstream
{
    AngelDumper dumper(m_script_engine);
    return dumper.dump();
}

auto StringSplit(std::string const& str, std::string_view seperator) -> std::vector<std::string_view>
{
    std::vector<std::string_view> ret{};

    std::size_t offset = 0;
    while (true) {
        auto pos = str.find(seperator, offset);
        if (pos == std::string::npos) {
            ret.push_back(std::string_view{ str.begin() + offset, str.end() });
            break;
        }
        ret.push_back(std::string_view{ str.begin() + offset, str.begin() + pos });

        offset = pos + 1;
    }
    return ret;
}

std::string_view StringTrimSpace(std::string_view& str)
{
    // Find the first non-whitespace character
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }

    // Find the last non-whitespace character
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }

    // Return the trimmed string_view
    return str.substr(start, end - start);
}

void ScriptingEngine::parse_attributes(CScriptBuilder& builder, ScriptAssembly& assembly)
{
    auto module = builder.GetModule();
    for (u32 i = 0; i < module->GetObjectTypeCount(); i++) {
        auto object = module->GetObjectTypeByIndex(i);

        (void)builder.GetMetadataForType(object->GetTypeId());
        // builder.GetMeta
    }

    (void)"Range(1, Awd(1, 1)), Editable(), Editable";
    (void)"Range(1, 2), Editable(), Editable";

    for (auto& [name, klass] : assembly.get_all_classes()) {
        for (auto& [prop_name, prop] : klass.m_properties) {
            auto attribute_sections = builder.GetMetadataForTypeProperty(klass.m_type->GetTypeId(), prop.index);
            for (auto text : attribute_sections) {
                SimpleLexer lexer(text);
                AttributeParser parser(lexer.scan());

                m_attributes[prop.uuid] = std::move(parser.parse());
            }
        }
    }
}

auto ScriptingEngine::compile_assembly(std::filesystem::path const& path, std::string const& module_name) -> Ref<ScriptAssembly>
{
    CScriptBuilder builder;
    auto r = builder.StartNewModule(m_script_engine, module_name.c_str());
    VERIFY(r >= 0, "Error while starting new module");

    for (auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (entry.is_directory() || entry.path().extension() != ".as")
            continue;

        r = builder.AddSectionFromFile(entry.path().string().c_str());
        if (r < 0) {
            LOG_ERRORF("Error while adding section from file: {}", entry.path().string());
        }
    }

    r = builder.BuildModule();
    if (r < 0) {
        LOG_ERRORF("Error while building module '{}'", module_name);
        return nullptr;
    }

    auto const module = m_script_engine->GetModule(module_name.c_str());

    if (m_loaded_assemblies.contains(module_name)) {
        m_loaded_assemblies[module_name]->reload(module);
    } else {
        m_loaded_assemblies[module_name] = make_ref<ScriptAssembly>(module);
    }

    parse_attributes(builder, *m_loaded_assemblies[module_name].get());

    return m_loaded_assemblies[module_name];
}

auto ScriptingEngine::compile_game_assembly(std::filesystem::path const& path) -> Ref<ScriptAssembly>
{
    return compile_assembly(path, "Game");
}

auto ScriptingEngine::get_game_assembly() -> Ref<ScriptAssembly>
{
    return m_loaded_assemblies["Game"];
}

// auto ScriptingEngine::GetAttribute(Uuid uuid) -> Scripting::Attribute*
// {
//     return s_Instance->m_Attributes[uuid].get();
// }
}
