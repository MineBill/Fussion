#include "EditorPCH.h"
#include "TextureSerializer.h"
#include "Project/Project.h"

#include <Fussion/Util/TextureImporter.h>

using namespace Fussion;

Ref<Asset> TextureSerializer::load(EditorAssetMetadata metadata)
{
    auto path = Project::assets_folder() / metadata.path;
    auto texture_metadata = std::dynamic_pointer_cast<Texture2DMetadata>(metadata.custom_metadata);

    auto ext = path.extension();
    if (ext == ".hdr") {
        auto [data, width, height] = TextureImporter::load_hdr_image_from_file(path).unwrap();
        texture_metadata->width = CAST(s32, width);
        texture_metadata->height = CAST(s32, height);
        texture_metadata->format = GPU::TextureFormat::RGBA32Float;
        texture_metadata->generate_mipmaps = false;
        return Texture2D::create(data, *texture_metadata);
    }

    auto [data, width, height] = TextureImporter::load_image_from_file(path).unwrap();
    texture_metadata->width = CAST(s32, width);
    texture_metadata->height = CAST(s32, height);
    return Texture2D::create(data, *texture_metadata);
}
