#include "TextureImporter.h"
#include "Engin5/Util/stb_image.h"

using namespace Engin5;

Ref<Texture2D> TextureImporter::LoadTextureFromFile(std::filesystem::path path)
{
    Texture2DSpec spec;
    int c;

    auto* data = stbi_load(path.string().c_str(), &spec.Width, &spec.Height, &c, 4);
    if (data == nullptr) {
        return nullptr;
    }
    return Texture2D::Create(data, spec);
}