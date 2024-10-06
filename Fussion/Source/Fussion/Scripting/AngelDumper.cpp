#include "AngelDumper.h"

#include <angelscript.h>
#include <iostream>
#include <iso646.h>

namespace Fussion {
    std::stringstream& AngelDumper::PrintEnumList(std::stringstream& out) const
    {
        for (u32 i = 0; i < m_ScriptEngine->GetEnumCount(); i++) {
            auto e = m_ScriptEngine->GetEnumByIndex(i);
            if (not e)
                continue;
            std::string_view const ns = e->GetNamespace();
            if (not ns.empty())
                out << std::format("namespace {} {{\n", ns);
            out << std::format("enum {} {{\n", e->GetName());
            for (u32 j = 0; j < e->GetEnumValueCount(); ++j) {
                out << std::format("\t{}", e->GetEnumValueByIndex(j, nullptr));
                if (j < e->GetEnumValueCount() - 1)
                    out << ",";
                out << "\n";
            }
            out << "}\n";
            if (not ns.empty())
                out << "}\n";
        }
        return out;
    }

    std::stringstream& AngelDumper::PrintClassTypeList(std::stringstream& out) const
    {
        for (u32 i = 0; i < m_ScriptEngine->GetObjectTypeCount(); i++) {
            auto const t = m_ScriptEngine->GetObjectTypeByIndex(i);
            if (not t)
                continue;

            std::string_view ns = t->GetNamespace();
            if (not ns.empty())
                out << std::format("namespace {} {{\n", ns);

            // Check if the type is a class or an interface
            if (t->GetFlags() & asOBJ_SCRIPT_OBJECT)
                out << "interface ";
            else
                out << "class ";
            out << t->GetName();

            if (t->GetSubTypeCount() > 0) {
                out << "<";
                for (u32 sub = 0; sub < t->GetSubTypeCount(); ++sub) {
                    auto const st = t->GetSubType(sub);
                    out << st->GetName();
                    if (sub < t->GetSubTypeCount() - 1)
                        out << ", ";
                }

                out << ">";
            }

            out << " {\n";
            for (u32 j = 0; j < t->GetBehaviourCount(); ++j) {
                asEBehaviours behaviours;
                auto const f = t->GetBehaviourByIndex(j, &behaviours);
                if (behaviours == asBEHAVE_CONSTRUCT
                    || behaviours == asBEHAVE_DESTRUCT || behaviours == asBEHAVE_FACTORY) {
                    out << std::format("\t{};\n", f->GetDeclaration(false, true, true));
                }
            }
            for (u32 j = 0; j < t->GetMethodCount(); ++j) {
                auto const m = t->GetMethodByIndex(j);
                out << std::format("\t{};\n", m->GetDeclaration(false, true, true));
            }
            for (u32 j = 0; j < t->GetPropertyCount(); ++j) {
                out << std::format("\t{};\n", t->GetPropertyDeclaration(j, true));
            }
            for (u32 j = 0; j < t->GetChildFuncdefCount(); ++j) {
                out << std::format("\tfuncdef {};\n", t->GetChildFuncdef(j)->GetFuncdefSignature()->GetDeclaration(false));
            }
            out << "}\n";
            if (not ns.empty())
                out << "}\n";
        }
        return out;
    }

    std::stringstream& AngelDumper::PrintGlobalFunctionList(std::stringstream& out) const
    {
        std::unordered_map<std::string, std::vector<asIScriptFunction*>> namespaced_global_functions;

        for (u32 i = 0; i < m_ScriptEngine->GetGlobalFunctionCount(); i++) {
            auto decl = m_ScriptEngine->GetGlobalFunctionByIndex(i);
            if (!decl)
                continue;

            std::string_view ns = decl->GetNamespace();
            if (ns.empty()) {
                out << std::format("{};\n", decl->GetDeclaration(false, false, true));
            } else {
                namespaced_global_functions[std::string(ns)].push_back(decl);
            }
        }

        for (auto const& [ns, functions] : namespaced_global_functions) {
            out << std::format("namespace {} {{\n", ns);

            for (auto const& decl : functions) {
                out << std::format("\t{};\n", decl->GetDeclaration(false, false, true));
            }

            out << "}\n";
        }

        return out;
    }

    std::stringstream& AngelDumper::PrintGlobalPropertyList(std::stringstream& out) const
    {
        struct Prop {
            std::string Name;
            std::string TypeDecl;
        };
        std::unordered_map<std::string, std::vector<Prop>> namespaced_global_props;

        for (u32 i = 0; i < m_ScriptEngine->GetGlobalPropertyCount(); i++) {
            char const* name;
            char const* ns0;
            int type;
            m_ScriptEngine->GetGlobalPropertyByIndex(i, &name, &ns0, &type, nullptr, nullptr, nullptr, nullptr);

            std::string t = m_ScriptEngine->GetTypeDeclaration(type, true);
            if (t.empty())
                continue;

            std::string_view ns = ns0;
            if (ns.empty()) {
                out << std::format("{} {};\n", t, name);
            } else {
                namespaced_global_props[std::string(ns)].push_back({ name, t });
            }
        }

        for (auto const& [ns, props] : namespaced_global_props) {
            out << std::format("namespace {} {{\n", ns);

            for (auto const& prop : props) {
                out << std::format("\t{} {};\n", prop.TypeDecl, prop.Name);
            }

            out << "}\n";
        }
        return out;
    }

    std::stringstream& AngelDumper::PrintGlobalTypedef(std::stringstream& out) const
    {
        for (u32 i = 0; i < m_ScriptEngine->GetTypedefCount(); ++i) {
            auto const type = m_ScriptEngine->GetTypedefByIndex(i);
            if (not type)
                continue;
            std::string_view const ns = type->GetNamespace();
            if (not ns.empty())
                out << std::format("namespace {} {{\n", ns);
            out << std::format(
                "typedef {} {};\n", m_ScriptEngine->GetTypeDeclaration(type->GetTypedefTypeId()), type->GetName());
            if (not ns.empty())
                out << "}\n";
        }
        return out;
    }

    std::stringstream AngelDumper::DumpTypes() const
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
