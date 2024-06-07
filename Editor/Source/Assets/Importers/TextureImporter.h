#pragma once
#include "Engin5/Assets/Texture2D.h"
#include "Engin5/Core/Types.h"

class TextureImporter
{
public:
    static Ref<Engin5::Texture2D> LoadTextureFromFile(std::filesystem::path path);
};
