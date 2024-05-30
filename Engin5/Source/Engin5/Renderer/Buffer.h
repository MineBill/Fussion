#pragma once
#include "RenderHandle.h"
#include "Engin5/Core/BitFlags.h"
#include "Engin5/Core/Types.h"

namespace Engin5
{
    enum class BufferUsage
    {
        Vertex = 1 << 0,
        Index = 1 << 1,
        Uniform = 1 << 2,
        TransferSource = 1 << 3,
        TransferDestination = 1 << 4,
    };

    DECLARE_FLAGS(BufferUsage, BufferUsageFlags)
    DECLARE_OPERATORS_FOR_FLAGS(BufferUsageFlags)

    struct BufferSpecification
    {
        std::string Label;
        BufferUsageFlags Usage;
        s32 Size;
        bool Mapped;
    };

    class Buffer: public RenderHandle
    {
    public:
        virtual void SetData(void* data, size_t size) = 0;

        virtual BufferSpecification const& GetSpec() const = 0;
    };
}