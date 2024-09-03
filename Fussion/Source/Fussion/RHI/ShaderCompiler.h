#pragma once
#include "Shader.h"
#include <Fussion/Core/Maybe.h>

namespace Fussion::RHI {
    class ShaderCompiler {
    public:
        enum class CompileError {};

        struct CompiledShader {
            /// Individual vertex, fragment, compute binaries.
            std::vector<ShaderStage> ShaderStages{};
            /// A single binary with all the stages linked together.
            std::vector<u32> LinkedStage{};
            ShaderMetadata Metadata{};
        };

        // TODO: Return some kind of error? The shaderc error api seems kind weird.
        /// Tries to compile a shader file source code.
        /// @returns The compilation result or nothing if the compilation failed.
        static auto Compile(std::string const& source_code, std::string const& file_name = "(memory buffer)") -> Maybe<CompiledShader>;
    };
}
