#pragma once
#include <Fussion/Core/Types.h>

namespace Fussion {
    struct Image {
        /// Data owns pixels in an RGBA format.
        std::vector<u8> Data{};
        u32 Width{}, Height{};

        bool IsValid() const
        {
            return !Data.empty();
        }

        u32 Size() const
        {
            constexpr u32 channels = 4;
            return Width * Height * channels;
        }
    };
}
