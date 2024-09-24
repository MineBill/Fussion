#include "AngelDumper.h"

#include <angelscript.h>
#include <iostream>
#include <iso646.h>

namespace Fussion {
    std::stringstream& AngelDumper::print_enum_list(std::stringstream& out) const
    {
        for (u32 i = 0; i < m_script_engine->GetEnumCount(); i++) {
            auto e = m_script_engine->GetEnumByIndex(i);
            if (not e)
                continue;
            const std::string_view ns = e->GetNamespace();
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

    std::stringstream& AngelDumper::print_class_type_list(std::stringstream& out) const
    {
        for (u32 i = 0; i < m_script_engine->GetObjectTypeCount(); i++) {
            const auto t = m_script_engine->GetObjectTypeByIndex(i);
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
                    if (sub < t->GetSubTypeCount() - 1)
                        out << ", ";
                    const auto st = t->GetSubType(sub);
                    out << st->GetName();
                }

                out << ">";
            }

            out << " {\n";
            for (u32 j = 0; j < t->GetBehaviourCount(); ++j) {
                asEBehaviours behaviours;
                const auto f = t->GetBehaviourByIndex(j, &behaviours);
                if (behaviours == asBEHAVE_CONSTRUCT
                    || behaviours == asBEHAVE_DESTRUCT || behaviours == asBEHAVE_FACTORY) {
                    out << std::format("\t{};\n", f->GetDeclaration(false, true, true));
                }
            }
            for (u32 j = 0; j < t->GetMethodCount(); ++j) {
                const auto m = t->GetMethodByIndex(j);
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

    std::stringstream& AngelDumper::print_global_function_list(std::stringstream& out) const
    {
        std::unordered_map<std::string, std::vector<asIScriptFunction*>> namespaced_global_functions;

        for (u32 i = 0; i < m_script_engine->GetGlobalFunctionCount(); i++) {
            auto decl = m_script_engine->GetGlobalFunctionByIndex(i);
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

    std::stringstream& AngelDumper::print_global_property_list(std::stringstream& out) const
    {
        struct Prop {
            std::string Name;
            std::string TypeDecl;
        };
        std::unordered_map<std::string, std::vector<Prop>> namespaced_global_props;

        for (u32 i = 0; i < m_script_engine->GetGlobalPropertyCount(); i++) {
            const char* name;
            const char* ns0;
            int type;
            m_script_engine->GetGlobalPropertyByIndex(i, &name, &ns0, &type, nullptr, nullptr, nullptr, nullptr);

            std::string t = m_script_engine->GetTypeDeclaration(type, true);
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

    std::stringstream& AngelDumper::print_global_typedef(std::stringstream& out) const
    {
        for (u32 i = 0; i < m_script_engine->GetTypedefCount(); ++i) {
            const auto type = m_script_engine->GetTypedefByIndex(i);
            if (not type)
                continue;
            const std::string_view ns = type->GetNamespace();
            if (not ns.empty())
                out << std::format("namespace {} {{\n", ns);
            out << std::format(
                "typedef {} {};\n", m_script_engine->GetTypeDeclaration(type->GetTypedefTypeId()), type->GetName());
            if (not ns.empty())
                out << "}\n";
        }
        return out;
    }

    std::stringstream AngelDumper::dump() const
    {
        std::stringstream stream;
        print_enum_list(stream);
        print_class_type_list(stream);
        print_global_function_list(stream);
        print_global_property_list(stream);
        print_global_typedef(stream);
        return stream;
    }
}
