#include "TextureImporter.h"
#include "Fussion/Util/stb_image.h"

using namespace Fussion;

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