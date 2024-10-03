#include "FussionPCH.h"
#include "Rect.h"

namespace Fussion {
    Rect::Rect(f32 x, f32 y, f32 width, f32 height)
        : position(x, y)
        , size(width, height)
    { }

    Rect::Rect(Vector2 const& position, Vector2 const& size)
        : position(position)
        , size(size)
    { }

    Rect Rect::FromSize(Vector2 const& size)
    {
        return { Vector2::Zero, size };
    }

    auto Rect::FromSize(f32 width, f32 height) -> Rect
    {
        return FromSize(Vector2(width, height));
    }

    Rect Rect::FromStartEnd(Vector2 const& start, Vector2 const& end)
    {
        return { start, end - start };
    }

    auto Rect::Contains(Vector2 const& point) const -> bool
    {
        auto end = position + size;
        auto top_left = point.x >= position.x && point.y >= position.y;
        auto bot_right = point.x < end.x && point.y < end.y;
        return top_left && bot_right;
    }
}
