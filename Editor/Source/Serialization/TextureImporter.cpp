#include "EditorPCH.h"

#include "Fussion/Util/ImageTools.h"
#include "Project/Project.h"
#include "TextureImporter.h"

#include <Fussion/Util/TextureLoader.h>

using namespace Fussion;

Ref<BinaryAsset> TextureImporter::import(std::filesystem::path const& path)
{
    // auto path = Project::AssetsFolderPath() / metadata.Path;
    // auto texture_metadata = std::dynamic_pointer_cast<Texture2DMetadata>(metadata.CustomMetadata);
    Texture2DMetadata texture_metadata;
    auto ext = path.extension();
    if (ext == ".hdr") {
        auto [data, width, height] = TextureLoader::load_hdr_image_from_file(path).unwrap();
        texture_metadata.width = CAST(s32, width);
        texture_metadata.height = CAST(s32, height);
        texture_metadata.format = GPU::TextureFormat::RGBA32Float;
        texture_metadata.generate_mipmaps = false;
        return Texture2D::create(data, texture_metadata);
    }

    auto image = TextureLoader::load_image_from_file(path).unwrap();
    texture_metadata.width = CAST(s32, image.width);
    texture_metadata.height = CAST(s32, image.height);
    texture_metadata.format = GPU::TextureFormat::RGBA8UnormSrgb;
    if (image.width % 4 == 0 && image.height % 4 == 0) {
        texture_metadata.format = GPU::TextureFormat::BC3RGBAUnormSrgb;
        auto compressed = ImageTools::compress_bc3(image);
        return Texture2D::create(compressed, texture_metadata);
    }
    return Texture2D::create(image.data, texture_metadata);
}
