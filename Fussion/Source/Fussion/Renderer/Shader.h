#pragma once
#include "RenderHandle.h"
#include "VertexLayout.h"
#include "Fussion/Core/BitFlags.h"

namespace Fussion
{
    struct ResourceUsage;
    class Pipeline;

    enum class ShaderType
    {
        None = 1 << 0,
        Vertex = 1 << 1,
        Fragment = 1 << 2,
        Compute = 1 << 3,
    };

    DECLARE_FLAGS(ShaderType, ShaderTypeFlags)
    DECLARE_OPERATORS_FOR_FLAGS(ShaderTypeFlags)

    struct ShaderUniform
    {
        std::string Name;
    };

    struct ShaderMetadata
    {
        std::vector<VertexAttribute> VertexAttributes;
        std::map<u32, std::map<u32, ResourceUsage>> Uniforms;
        std::vector<u32> ColorOutputs;
    };

    struct ShaderStage
    {
        ShaderType Type;

        std::vector<u32> Bytecode;
    };

    class Shader: public RenderHandle
    {
    public:
        virtual Ref<Pipeline> GetPipeline() const = 0;
        virtual ShaderMetadata const& GetMetadata() = 0;
    };
}