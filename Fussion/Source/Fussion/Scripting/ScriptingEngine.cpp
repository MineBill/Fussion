#include "e5pch.h"
#include "Fussion/Scripting/ScriptingEngine.h"

#include "Fussion/Scripting/AngelDumper.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Log/Log.h"

#include "scriptstdstring/scriptstdstring.h"
#include "scriptbuilder/scriptbuilder.h"
#include "scriptarray/scriptarray.h"
#include "scriptany/scriptany.h"

namespace Fussion
{
    ScriptingEngine* ScriptingEngine::s_Instance = nullptr;

    void MessageCallback(const asSMessageInfo *msg, [[maybe_unused]] void *param)
    {
        if (msg->type == asMSGTYPE_ERROR)
            LOG_ERRORF("{}({}, {}): {}", msg->section, msg->row, msg->col, msg->message);
        else if( msg->type == asMSGTYPE_WARNING )
            LOG_WARNF("{}({}, {}): {}", msg->section, msg->row, msg->col, msg->message);
        else if( msg->type == asMSGTYPE_INFORMATION )
            LOG_INFOF("{}({}, {}): {}", msg->section, msg->row, msg->col, msg->message);
    }

    void Print(std::string const& message)
    {
        LOG_INFOF("[Script]: {}", message);
    }

    void LogDebug(std::string const& message, [[maybe_unused]] ScriptingEngine* engine)
    {
        LOG_DEBUGF("[Script]: {}", message);
    }


    ScriptingEngine::ScriptingEngine()
    {
        m_ScriptEngine = asCreateScriptEngine();

        auto r = m_ScriptEngine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);
        VERIFY(r >= 0, "Failed to set message callback");

        RegisterStdString(m_ScriptEngine);
        RegisterScriptArray(m_ScriptEngine, false);
        m_ScriptEngine->RegisterDefaultArrayType("Array<T>");
        RegisterScriptAny(m_ScriptEngine);

        m_GameModule = m_ScriptEngine->GetModule("GameModule", asGM_ALWAYS_CREATE);

        m_ScriptEngine->RegisterGlobalFunction("void Print(const string &in message)", asFUNCTION(Print), asCALL_CDECL);

        {
            auto default_namespace = m_ScriptEngine->GetDefaultNamespace();
            defer (m_ScriptEngine->SetDefaultNamespace(default_namespace));

            m_ScriptEngine->SetDefaultNamespace("Log");
            m_ScriptEngine->RegisterGlobalFunction("void Debug(const string &in message)", asFUNCTION(LogDebug), asCALL_CDECL, this);
        }

        m_ScriptEngine->RegisterInterface("Script");
    }

    void ScriptingEngine::Initialize()
    {
        s_Instance = new ScriptingEngine;
    }

    void ScriptingEngine::Shutdown()
    {
        delete s_Instance;
        s_Instance = nullptr;
    }

    void ScriptingEngine::LoadScriptFromPath(std::filesystem::path path)
    {
        // m_GameModule->LoadByteCode()
    }

    std::stringstream ScriptingEngine::DumpCurrentTypes() const
    {
        AngelDumper dumper(m_ScriptEngine);
        return dumper.Dump();
    }

    Ref<ScriptAssembly> ScriptingEngine::CompileAssembly(std::filesystem::path path, std::string const& module_name)
    {
        CScriptBuilder builder;
        auto r = builder.StartNewModule(m_ScriptEngine, module_name.c_str());
        VERIFY(r >= 0, "Error while starting new module");

        for (auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_directory() || entry.path().extension() != ".as") continue;

            r = builder.AddSectionFromFile(entry.path().string().c_str());
            VERIFY(r >= 0, "Error while adding section from file: {}", entry.path().string());
        }

        r = builder.BuildModule();
        VERIFY(r >= 0, "Error while building game module");

        auto const module = m_ScriptEngine->GetModule(module_name.c_str());
        m_LoadedAssemblies[module_name] = MakeRef<ScriptAssembly>(module);
        return m_LoadedAssemblies[module_name];
    }
}