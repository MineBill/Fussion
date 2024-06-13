#pragma once
#include "Fussion/Assets/Texture2D.h"
#include "Fussion/Core/Types.h"

class TextureImporter
{
public:
    static Ref<Fussion::Texture2D> LoadTextureFromFile(std::filesystem::path path);
};
