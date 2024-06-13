#pragma once
#include <angelscript.h>
#include <sstream>

namespace Fussion
{
    class AngelDumper
    {
    public:
        explicit AngelDumper(asIScriptEngine const* script_engine): m_ScriptEngine(script_engine) {}

        std::stringstream& PrintEnumList(std::stringstream&) const;
        std::stringstream& PrintClassTypeList(std::stringstream&) const;
        std::stringstream& PrintGlobalFunctionList(std::stringstream&) const;
        std::stringstream& PrintGlobalPropertyList(std::stringstream&) const;
        std::stringstream& PrintGlobalTypedef(std::stringstream&) const;

        std::stringstream Dump() const;
    private:
        asIScriptEngine const* m_ScriptEngine;
    };
}
