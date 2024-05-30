#pragma once
#include "Shader.h"

namespace Engin5
{
    class ShaderCompiler
    {
    public:
        static auto Compile(std::string const& source_code) -> std::tuple<std::vector<ShaderStage>, ShaderMetadata>;
    };
}
