#pragma once
#include "Fussion/GPU/GPU.h"

namespace Fussion::GPU {
    class ShaderProcessor {
    public:
        /// Process shader file and resolve custom directives, such as '#import'.
        static auto ProcessFile(std::filesystem::path const& path) -> Maybe<std::string>;

        struct ShaderUniform {
            std::string Name {};
        };

        struct PushConstant {
            ShaderStage Stage {};
            std::string Name {};
            size_t Size {};
        };

        struct ParsedPragma {
            std::string Key {};
            std::string Value {};
        };

        struct ResourceUsage {
            std::string Label = "Resource Usage";
            BindingType::Type Type;
            s32 Count = 1;
            ShaderStageFlags Stages;
            u32 Binding = 42069;
        };

        using BindingSet = u32;
        using BindingIndex = u32;

        struct ShaderMetadata {
            std::vector<VertexAttribute> VertexAttributes {};
            std::map<BindingSet, std::map<BindingIndex, ResourceUsage>> Uniforms {};
            std::vector<u32> ColorOutputs {};
            std::vector<PushConstant> PushConstants {};
            std::vector<ParsedPragma> ParsedPragmas {};
            bool UseBlending {};
            bool UseDepth { true };
            u32 Samples { 1 };
            Maybe<DepthStencilState> DepthState {};
        };

        struct CompiledShaderStage {
            ShaderStage Type {};
            std::vector<u32> Bytecode {};
        };

        struct CompiledShader {
            /// Individual vertex, fragment, compute binaries.
            std::vector<CompiledShaderStage> ShaderStages {};
            /// A single binary with all the stages linked together.
            std::vector<u32> LinkedStage {};
            std::vector<u32> VertexStage {};
            std::vector<u32> FragmentStage {};
            ShaderMetadata Metadata {};
        };

        /// Loads and compiles a Slang shader.
        /// @param path The path of the shader file.
        static auto CompileSlang(std::filesystem::path const& path) -> Maybe<CompiledShader>;
    };
}
