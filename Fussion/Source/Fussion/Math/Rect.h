﻿#pragma once
#include "Vector2.h"

#include <Fussion/Core/Types.h>
#include <Fussion/Log/Formatters.h>

namespace Fussion {
    struct Rect {
        Vector2 position {};
        Vector2 size {};

        Rect(f32 x, f32 y, f32 width, f32 height);
        Rect(Vector2 const& position, Vector2 const& size);

        /// Create a Rect only from size. The position is set to (0, 0).
        /// @return The rect.
        static Rect FromSize(Vector2 const& size);

        /// Create a Rect only from size. The position is set to (0, 0).
        /// @return The rect.
        static auto FromSize(f32 width, f32 height) -> Rect;

        /// Create a rect from two positions. The size is calculated automatically.
        /// @return The rect.
        static Rect FromStartEnd(Vector2 const& start, Vector2 const& end);

        /// Checks whether @p point is contained within this rect.
        auto Contains(Vector2 const& point) const -> bool;
    };
}

FSN_MAKE_FORMATTABLE(Fussion::Rect, "Rect({}, {})", v.position, v.size)
