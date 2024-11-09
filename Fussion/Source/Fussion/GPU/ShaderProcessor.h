#pragma once
#include "Fussion/GPU/GPU.h"

namespace Fussion::GPU {
    class ShaderProcessor {
    public:
        /// Process shader file and resolve custom directives, such as '#import'.
        static auto process_file(std::filesystem::path const& path) -> Maybe<std::string>;

        struct ShaderUniform {
            std::string name {};
        };

        struct PushConstant {
            ShaderStage stage {};
            std::string name {};
            size_t size {};
        };

        struct ParsedPragma {
            std::string key {};
            std::string value {};
        };

        struct ResourceUsage {
            std::string label = "Resource Usage";
            BindingType::Type type;
            s32 count = 1;
            ShaderStageFlags stages;
            u32 binding = 42069;
        };

        using BindingSet = u32;
        using BindingIndex = u32;

        struct ShaderMetadata {
            std::vector<VertexAttribute> vertex_attributes {};
            std::map<BindingSet, std::map<BindingIndex, ResourceUsage>> uniforms {};
            std::vector<u32> color_outputs {};
            std::vector<PushConstant> push_constants {};
            std::vector<ParsedPragma> parsed_pragmas {};
            bool use_blending {};
            bool use_depth { true };
            u32 samples { 1 };
            Maybe<DepthStencilState> depth_state {};
        };

        struct CompiledShaderStage {
            ShaderStage type {};
            std::vector<u32> bytecode {};
        };

        struct CompiledShader {
            /// Individual vertex, fragment, compute binaries.
            std::vector<CompiledShaderStage> shader_stages {};
            /// A single binary with all the stages linked together.
            std::vector<u32> linked_stage {};
            std::vector<u32> vertex_stage {};
            std::vector<u32> fragment_stage {};
            ShaderMetadata metadata {};
        };

        static void initialize();
        static void shutdown();

        /// Loads and compiles a Slang shader.
        /// @param path The path of the shader file.
        static auto compile_slang(std::filesystem::path const& path) -> Maybe<CompiledShader>;
    };
}
