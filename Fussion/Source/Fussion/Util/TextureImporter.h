#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Types.h"
#include <Fussion/Image.h>

namespace Fussion {
    class TextureImporter {
    public:
        static auto load_image_from_memory(std::span<u8> data) -> Maybe<Image>;
        static auto load_image_from_file(std::filesystem::path const& path) -> Maybe<Image>;
        static auto load_texture_from_file(std::filesystem::path const& path) -> Maybe<Ref<Texture2D>>;
        static auto load_texture_from_memory(std::span<u8> data, bool is_normal_map = false) -> Maybe<Ref<Texture2D>>;

        static void save_image_to_file(GPU::Texture const& texture, std::filesystem::path const& path);
    };
}
