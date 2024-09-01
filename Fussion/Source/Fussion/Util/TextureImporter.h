#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Types.h"
#include <Fussion/Image.h>

namespace Fussion {
    class TextureImporter {
    public:
        static auto LoadImageFromMemory(std::span<u8> data) -> Image;
        static auto LoadImageFromFile(std::filesystem::path const& path) -> Image;
        static auto LoadTextureFromFile(std::filesystem::path const& path) -> Ref<Texture2D>;
        static auto LoadTextureFromMemory(std::span<u8> data, bool is_normal_map = false) -> Ref<Texture2D>;

        static void SaveImageToFile(Ref<RHI::Image> const& image, std::filesystem::path const& path);
    };
}
