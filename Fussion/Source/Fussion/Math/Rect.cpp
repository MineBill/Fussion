#include "W:\source\projects\Fussion\build\.gens\Fussion\windows\x64\debug\Fussion\Source\e5pch.h"
#include "Rect.h"

namespace Fussion {
    Rect::Rect(f32 x, f32 y, f32 width, f32 height)
        : Position(x, y), Size(width, height) {}

    Rect::Rect(Vector2 const& position, Vector2 const& size)
        : Position(position), Size(size) {}

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
        auto end = Position + Size;
        auto top_left = point.X >= Position.X && point.Y >= Position.Y;
        auto bot_right = point.X < end.X && point.Y < end.Y;
        return top_left && bot_right;
    }
}
