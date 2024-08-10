#pragma once
#include "Shader.h"
#include <Fussion/Core/Maybe.h>

namespace Fussion::RHI {
    class ShaderCompiler {
    public:
        enum class CompileError {};

        struct CompileResult {
            std::vector<ShaderStage> ShaderStages{};
            ShaderMetadata Metadata{};
        };

        // TODO: Return some kind of error? The shaderc error api seems kind weird.
        /// Tries to compile a shader file source code.
        /// @returns The compilation result or nothing if the compilation failed.
        static auto Compile(std::string const& source_code, std::string const& file_name = "(memory buffer)") -> Maybe<CompileResult>;
    };
}
