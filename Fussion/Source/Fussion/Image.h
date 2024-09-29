#pragma once
#include <Fussion/Core/Types.h>

namespace Fussion {
    struct Image {
        /// Data owns pixels in an RGBA format.
        std::vector<u8> data{};
        u32 width{}, height{};

        bool is_valid() const
        {
            return !data.empty();
        }

        u32 size() const
        {
            constexpr u32 channels = 4;
            return width * height * channels;
        }
    };

    struct FloatImage {
        /// Data owns pixels in an RGBA format.
        std::vector<f32> data{};
        u32 width{}, height{};

        bool is_valid() const
        {
            return !data.empty();
        }

        u32 size() const
        {
            constexpr u32 channels = 4;
            return width * height * channels;
        }
    };
}
