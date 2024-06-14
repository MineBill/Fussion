#include "Reflect/CodeGenerate/CG_Utils.h"
#include <fstream>

namespace Reflect::CodeGeneration
{
    std::string CG_Utils::WriteReflectTypeCPPDeclare(std::string_view type)
    {
        std::string str;
        str += "::Reflect::ReflectTypeCPP<"; str += type; str += ">";
        return str;
    }

    std::string CG_Utils::WriteReflectTypeCPPParentheses(EReflectType reflectType, EReflectValueType valueType, const std::vector<Parser::ReflectInheritanceData>& inheritance, std::string_view name, std::string_view instanceChaiPrefix)
    {
        std::string str;
        str += "(";
        str += "static_cast<::Reflect::EReflectType>(" + std::to_string(static_cast<int>(reflectType)) + "), ";
        str += "static_cast<::Reflect::EReflectValueType>(" + std::to_string(static_cast<int>(valueType)) + "), ";
        if (!inheritance.empty())
        {
            str += "std::move(";
            if (!instanceChaiPrefix.empty())
            {
                str += std::string(instanceChaiPrefix) + "_";
            }
            str += std::string(name) + "_InheritanceChain), ";
        }
        else
        {
            str += "std::vector<std::unique_ptr<::Reflect::ReflectType>>(), ";
        }
        //str += "Reflect::vector_from_il<std::unique_ptr<::Reflect::ReflectType>>(\n";
        //{
        //    str += "\t{\n";
        //    for (const Parser::ReflectInheritanceData& type : inheritance)
        //    {
        //        str += "\t\tstd::make_unique<" + WriteReflectTypeCPPDeclare(type.NameWithNamespace) + ">" + WriteReflectTypeCPPParentheses(reflectType, EReflectValueType::Unknown, type.Inheritances, type.Name) + ", \n";
        //    }
        //    str += "\t}\n";
        //}
        //str += "), \n";
        str += "\""; str += name; str += "\"";
        str += ")";
        return str;
    }

    std::string CG_Utils::WriteReflectTypeCPPMember(Parser::ReflectMemberData memberData)
    {
        return WriteReflectTypeCPP(memberData.Type, EReflectType::Member, memberData.ReflectValueType, memberData.TypeInheritance, memberData.Name);
    }

    std::string CG_Utils::WriteReflectTypeCPPFunction(Parser::ReflectFunctionData functionData)
    {
        return WriteReflectTypeCPP(functionData.Type, EReflectType::Function, functionData.ReflectValueType, functionData.TypeInheritance, functionData.Name);
    }

    std::string CG_Utils::WriteReflectTypeCPPParameter(Parser::ReflectTypeNameData parameterData)
    {
        return WriteReflectTypeCPP(parameterData.Type, EReflectType::Parameter, parameterData.ReflectValueType, parameterData.TypeInheritance, parameterData.Name);
    }

    void CG_Utils::WriteIfDefines(const Parser::ReflectContainerData& data, std::ofstream& file)
    {
        for (std::string define : data.IfDefines)
        {
            file << define << '\n';
        }
    }

    void CG_Utils::WriteEndIfDefines(const Parser::ReflectContainerData& data, std::ofstream& file)
    {
        for (const std::string& define : data.IfDefines)
        {
            file << "#endif" << '\n';
        }
    }

    void CG_Utils::PushDisableWarnings(std::ofstream& file)
    {
        file << "#if defined(_MSC_VER)" << '\n';
        file << "#pragma warning(push)" << '\n';
        // Disable warnings for unused parameters and variables
        // 4100: unreferenced formal parameter
        // 4101: unreferenced local variable
        // 4189: local variable is initialized but not referenced
        // 4464: relative include path contains '..'
        // 4061: enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
        //       This would be a cool warning to have enabled, but shit plus plus doesn't allow you to opt out for
        //       certain switch cases, so ¯\_(ツ)_/¯.
        for (const std::string& warning : {"4100", "4101", "4189", "4464", "4061"})
        {
            file << "#pragma warning(disable: " << warning << ")" << '\n';
        }
        file << "#elif defined(__GNUC__) || defined(__clang__)" << '\n';
        file << "#pragma GCC diagnostic push" << '\n';
        for (const std::string& warning : {"unused-parameter", "unused-variable", "endif-labels"})
        {
            file << "#pragma GCC diagnostic ignored \"-W" << warning << "\"" << '\n';
        }
        file << "#endif" << '\n';
    }

    void CG_Utils::PopDisableWarnings(std::ofstream& file)
    {
        file << "#if defined(_MSC_VER)" << '\n';
        file << "#pragma warning(pop)" << '\n';
        file << "#elif defined(__GNUC__) || defined(__clang__)" << '\n';
        file << "#pragma GCC diagnostic pop" << '\n';
        file << "#endif" << '\n';
    }

    std::string CG_Utils::WriteReflectTypeCPP(std::string_view type, EReflectType reflectType, EReflectValueType valueType, const std::vector<Parser::ReflectInheritanceData>& inheritance, std::string_view name)
    {
        std::string str;
        str += WriteReflectTypeCPPDeclare(type);
        str += WriteReflectTypeCPPParentheses(reflectType, valueType, inheritance, name);
        return str;
    }
}