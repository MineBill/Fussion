#include "AngelDumper.h"

#include <angelscript.h>
#include <format>
#include <iostream>
#include <iso646.h>

namespace Engin5
{
    std::stringstream& AngelDumper::PrintEnumList(std::stringstream& out) const
    {
        for (int i = 0; i < m_ScriptEngine->GetEnumCount(); i++)
        {
            const auto e = m_ScriptEngine->GetEnumByIndex(i);
            if (not e) continue;
            const std::string_view ns = e->GetNamespace();
            if (not ns.empty()) out << std::format("namespace {} {{\n", ns);
            out << std::format("enum {} {{\n", e->GetName());
            for (int j = 0; j < e->GetEnumValueCount(); ++j)
            {
                out << std::format("\t{}", e->GetEnumValueByIndex(j, nullptr));
                if (j < e->GetEnumValueCount() - 1) out << ",";
                out << "\n";
            }
            out << "}\n";
            if (not ns.empty()) out << "}\n";
        }
        return out;
    }

    std::stringstream& AngelDumper::PrintClassTypeList(std::stringstream& out) const
    {
        for (int i = 0; i < m_ScriptEngine->GetObjectTypeCount(); i++)
        {
            const auto t = m_ScriptEngine->GetObjectTypeByIndex(i);
            if (not t) continue;

            const std::string_view ns = t->GetNamespace();
            if (not ns.empty()) out << std::format("namespace {} {{\n", ns);

            // Check if the type is a class or an interface
            if (t->GetFlags() & asOBJ_SCRIPT_OBJECT)
                out << "interface ";
            else
                out << "class ";
            out << t->GetName();

            if (t->GetSubTypeCount() > 0)
            {
                out << "<";
                for (int sub = 0; sub < t->GetSubTypeCount(); ++sub)
                {
                    if (sub < t->GetSubTypeCount() - 1) out << ", ";
                    const auto st = t->GetSubType(sub);
                    out << st->GetName();
                }

                out << ">";
            }

            out << "{\n";
            for (int j = 0; j < t->GetBehaviourCount(); ++j)
            {
                asEBehaviours behaviours;
                const auto f = t->GetBehaviourByIndex(j, &behaviours);
                if (behaviours == asBEHAVE_CONSTRUCT
                    || behaviours == asBEHAVE_DESTRUCT)
                {
                    out << std::format("\t{};\n", f->GetDeclaration(false, true, true));
                }
            }
            for (int j = 0; j < t->GetMethodCount(); ++j)
            {
                const auto m = t->GetMethodByIndex(j);
                out << std::format("\t{};\n", m->GetDeclaration(false, true, true));
            }
            for (int j = 0; j < t->GetPropertyCount(); ++j)
            {
                out << std::format("\t{};\n", t->GetPropertyDeclaration(j, true));
            }
            for (int j = 0; j < t->GetChildFuncdefCount(); ++j)
            {
                out << std::format("\tfuncdef {};\n", t->GetChildFuncdef(j)->GetFuncdefSignature()->GetDeclaration(false));
            }
            out << "}\n";
            if (not ns.empty()) out << "}\n";
        }
        return out;
    }

    std::stringstream& AngelDumper::PrintGlobalFunctionList(std::stringstream& out) const
    {
        for (int i = 0; i < m_ScriptEngine->GetGlobalFunctionCount(); i++)
        {
            const auto f = m_ScriptEngine->GetGlobalFunctionByIndex(i);
            if (not f) continue;
            const std::string_view ns = f->GetNamespace();
            if (not ns.empty()) out << std::format("namespace {} {{ ", ns);
            out << std::format("{};", f->GetDeclaration(false, false, true));
            if (not ns.empty()) out << " }";
            out << "\n";
        }
        return out;
    }

    std::stringstream& AngelDumper::PrintGlobalPropertyList(std::stringstream& out) const
    {
        for (int i = 0; i < m_ScriptEngine->GetGlobalPropertyCount(); i++)
        {
            const char* name;
            const char* ns0;
            int type;
            m_ScriptEngine->GetGlobalPropertyByIndex(i, &name, &ns0, &type, nullptr, nullptr, nullptr, nullptr);

            const std::string t = m_ScriptEngine->GetTypeDeclaration(type, true);
            if (t.empty()) continue;

            std::string_view ns = ns0;
            if (not ns.empty()) out << std::format("namespace {} {{ ", ns);

            out << std::format("{} {};", t, name);
            if (not ns.empty()) out << " }";
            out << "\n";
        }
        return out;
    }

    std::stringstream& AngelDumper::PrintGlobalTypedef(std::stringstream& out) const
    {
        for (int i = 0; i < m_ScriptEngine->GetTypedefCount(); ++i)
        {
            const auto type = m_ScriptEngine->GetTypedefByIndex(i);
            if (not type) continue;
            const std::string_view ns = type->GetNamespace();
            if (not ns.empty()) out << std::format("namespace {} {{\n", ns);
            out << std::format(
                "typedef {} {};\n", m_ScriptEngine->GetTypeDeclaration(type->GetTypedefTypeId()), type->GetName());
            if (not ns.empty()) out << "}\n";
        }
        return out;
    }

    std::stringstream AngelDumper::Dump() const
    {
        std::stringstream stream;
        PrintEnumList(stream);
        PrintClassTypeList(stream);
        PrintGlobalFunctionList(stream);
        PrintGlobalPropertyList(stream);
        PrintGlobalTypedef(stream);
        return stream;
    }
}