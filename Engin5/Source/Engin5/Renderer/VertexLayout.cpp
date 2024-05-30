#include "VertexLayout.h"

namespace Engin5
{
    VertexAttributeLayout::VertexAttributeLayout(std::span<VertexAttribute> attributes)
    {
        s32 offset = 0;
        for (auto& attr : attributes) {
            attr.Offset = offset;
            Attributes.push_back(attr);

            offset += ElementTypeCount(attr.Type) * 4;
        }
        Stride = offset;
    }

    VertexAttributeLayout VertexAttributeLayout::Create(std::span<VertexAttribute> attributes)
    {
        return VertexAttributeLayout(attributes);
    }
}