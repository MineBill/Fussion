#pragma once
#include "Fussion/Core/Types.h"

namespace Fussion
{
    enum class ElementType
    {
        Int,
        Int2,
        Int3,
        Int4,
        Float,
        Float2,
        Float3,
        Float4,
        Mat3,
        Mat4,
    };

    inline s32 ElementTypeCount(const ElementType type)
    {
        switch (type) {
        using enum ElementType;
        case Int:
        case Float:
            return 1;
        case Int2:
        case Float2:
            return 2;
        case Int3:
        case Float3:
            return 3;
        case Int4:
        case Float4:
            return 4;
        case Mat3:
            return 3 * 3;
        case Mat4:
            return 4 * 4;
        }
        return 0;
    }

    struct VertexAttribute
    {
        std::string Name;
        ElementType Type;

        s32 Offset{};
    };

    class VertexAttributeLayout
    {
        explicit VertexAttributeLayout(std::span<VertexAttribute> attributes);
    public:
        VertexAttributeLayout() = default;

        static VertexAttributeLayout Create(std::span<VertexAttribute> attributes);

        std::vector<VertexAttribute> Attributes{};
        s32 Stride{};
    };
}