#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Types.h"
#include <Fussion/Image.h>

class TextureImporter {
public:
    static auto LoadImageFromMemory(std::span<u8> data) -> Fussion::Image;
    static auto LoadImageFromFile(std::filesystem::path const& path) -> Fussion::Image;
    static auto LoadTextureFromFile(std::filesystem::path const& path) -> Ref<Fussion::Texture2D>;
};
