#pragma once
#include <Fussion/Image.h>

namespace Fussion {
    class ImageTools final {
    public:
        /// Initializes the ImageTools class.
        /// This is required before any compression or decompression can be performed.
        static void initialize();
        /// Takes as input an image and compresses it using BC3.
        /// @returns A newly allocated image.
        static auto compress_bc3(Image const& image) -> std::vector<u8>;
    };
}
