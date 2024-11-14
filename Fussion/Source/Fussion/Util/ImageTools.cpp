#include "ImageTools.h"
#include "rgbcx.h"

namespace Fussion {

    void ImageTools::initialize()
    {
        rgbcx::init();
    }

    auto ImageTools::compress_bc3(Image const& image) -> std::vector<u8>
    {
        u32 blocks_x = image.width / 4;  // 640 blocks
        u32 blocks_y = image.height / 4; // 360 blocks

        std::vector<u8> compressed_image {};
        compressed_image.resize(blocks_x * blocks_y * 16);

        auto pixel_ptr = image.data.data();
        for (u32 y = 0; y < blocks_y; y++) {
            for (u32 x = 0; x < blocks_x; x++) {
                u8 input[64] {};

                auto get = [&](u32 xx, u32 yy) {
                    return &pixel_ptr[(xx + image.width * yy) * 4];
                };

                for (u32 yy = 0; yy < 4; yy++) {
                    Mem::copy(input + yy * 16, get(x * 4, y * 4 + yy), 16);
                }

                rgbcx::encode_bc3(
                    2,
                    &compressed_image[(x + y * blocks_x) * 16],
                    input
                );
            }
        }
        return compressed_image;
    }
}
