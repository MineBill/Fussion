#pragma once
#include "RenderHandle.h"
#include "VertexLayout.h"
#include "Resources/Resource.h"
#include "Fussion/RHI/ShaderFlags.h"

#include <map>

namespace Fussion::RHI {
    class Pipeline;

    struct ShaderUniform {
        std::string Name{};
    };

    struct PushConstant {
        ShaderType Stage{};
        std::string Name{};
        size_t Size{};
    };

    struct ParsedPragma {
        std::string Key{};
        std::string Value{};
    };

    struct ShaderMetadata {
        std::vector<VertexAttribute> VertexAttributes{};
        std::map<u32, std::map<u32, ResourceUsage>> Uniforms{};
        std::vector<u32> ColorOutputs{};
        std::vector<PushConstant> PushConstants{};
        std::vector<ParsedPragma> ParsedPragmas{};
        bool UseBlending{};
        u32 Samples{ 1 };
    };

    struct ShaderStage {
        ShaderType Type{};

        std::vector<u32> Bytecode{};
    };

    class Shader : public RenderHandle {
    public:
        virtual Ref<Pipeline> GetPipeline() const = 0;
        virtual ShaderMetadata const& GetMetadata() = 0;
    };
}
