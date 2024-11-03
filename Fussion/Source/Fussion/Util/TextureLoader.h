#pragma once
#include <Fussion/Assets/Texture2D.h>
#include <Fussion/Core/Span.h>
#include <Fussion/Core/Types.h>
#include <Fussion/Image.h>

namespace Fussion {
    class TextureLoader {
    public:
        static auto LoadImageFromMemory(ReadOnlySpan<u8> data) -> Maybe<Image>;
        static auto LoadImageFromFile(std::filesystem::path const& path) -> Maybe<Image>;
        static auto LoadTextureFromFile(std::filesystem::path const& path) -> Maybe<Ref<Texture2D>>;
        static auto LoadTextureFromMemory(ReadOnlySpan<u8> data, bool is_normal_map = false) -> Maybe<Ref<Texture2D>>;

        static void SaveImageToFile(GPU::Texture const& texture, std::filesystem::path const& path);

        static auto LoadHDRImageFromMemory(ReadOnlySpan<u8> data) -> Maybe<FloatImage>;
        static auto LoadHDRImageFromFile(std::filesystem::path const& path) -> Maybe<FloatImage>;
        static auto LoadHDRTextureFromMemory(ReadOnlySpan<u8> data, bool is_normal_map = false) -> Maybe<Ref<Texture2D>>;
    };
}
