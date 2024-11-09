#include "EditorPCH.h"

#include "Project/Project.h"
#include "TextureImporter.h"

#include <Fussion/Util/TextureLoader.h>

using namespace Fussion;

Ref<Asset> TextureImporter::Import(std::filesystem::path const& path)
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

    auto [data, width, height] = TextureLoader::load_image_from_file(path).unwrap();
    texture_metadata.width = CAST(s32, width);
    texture_metadata.height = CAST(s32, height);
    return Texture2D::create(data, texture_metadata);
}
