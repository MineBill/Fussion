#pragma once
#include <angelscript.h>
#include <sstream>

namespace Fussion
{
    class AngelDumper
    {
    public:
        explicit AngelDumper(asIScriptEngine const* script_engine): m_script_engine(script_engine) {}

        std::stringstream& print_enum_list(std::stringstream&) const;
        std::stringstream& print_class_type_list(std::stringstream&) const;
        std::stringstream& print_global_function_list(std::stringstream&) const;
        std::stringstream& print_global_property_list(std::stringstream&) const;
        std::stringstream& print_global_typedef(std::stringstream&) const;

        std::stringstream dump() const;
    private:
        asIScriptEngine const* m_script_engine;
    };
}
