#pragma once

#include "Reflect/FileParser/ParserStructs.h"

namespace Reflect
{
    struct ReflectAdditionalOptions;
}

namespace Reflect::CodeGeneration
{
#ifdef REFLECT_TYPE_INFO_ENABLED
    class CG_CPP_TypeInfo
    {
    public:
        
        void Generate(const Parser::ReflectContainerData& data, std::ofstream& file, const ReflectAdditionalOptions* additionalOptions);

    private:
        void WriteGenerateTypeInfo(const Parser::ReflectContainerData& data, std::ofstream& file, const ReflectAdditionalOptions* additionalOptions);
        void WriteGenerateTypeInheritance(const Parser::ReflectContainerData& data, std::ofstream& file, const ReflectAdditionalOptions* additionalOptions);
        void WriteGenerateTypeMembers(const Parser::ReflectContainerData& data, std::ofstream& file, const ReflectAdditionalOptions* additionalOptions);
        void WriteGenerateTypeFunctions(const Parser::ReflectContainerData& data, std::ofstream& file, const ReflectAdditionalOptions* additionalOptions);

        void WriteReflectCPPInheritanceChain(std::ofstream& file, EReflectType refectType, const Parser::ReflectTypeNameData& typeNameData, int lineIndent);

        std::string GetTypeName(const Parser::ReflectContainerData& data) const;
    };
#endif
}