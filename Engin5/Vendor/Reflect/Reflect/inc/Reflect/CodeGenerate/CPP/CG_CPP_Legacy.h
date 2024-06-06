#pragma once

#include "Reflect/Core/Defines.h"
#include "Reflect/FileParser/ParserStructs.h"

namespace Reflect
{
    struct ReflectAdditionalOptions;
}

namespace Reflect::CodeGeneration
{
    class CG_CPP_Legacy
    {
    public:

        void Generate(const Parser::ReflectContainerData& data, std::ofstream& file, const ReflectAdditionalOptions* additionalOptions);

    private:
        void WriteMemberProperties(const Parser::ReflectContainerData& data, std::ofstream& file, const ReflectAdditionalOptions* additionalOptions);

        void WriteMemberGet(const Parser::ReflectContainerData& data, std::ofstream& file, const ReflectAdditionalOptions* additionalOptions);
        void WriteFunctionGet(const Parser::ReflectContainerData& data, std::ofstream& file, const ReflectAdditionalOptions* additionalOptions);

        void WriteReflectCPPInheritanceChain(std::ofstream& file, EReflectType reflectType, const Parser::ReflectTypeNameData& typeNameData, int indent);

        std::string MemberFormat();
    };
}