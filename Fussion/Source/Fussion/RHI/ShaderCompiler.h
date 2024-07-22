#pragma once
#include "Shader.h"

namespace Fussion::RHI {
class ShaderCompiler {
public:
    static auto Compile(std::string const& source_code) -> std::tuple<std::vector<ShaderStage>, ShaderMetadata>;
};
}
