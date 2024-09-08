#include "EditorPCH.h"
#include "TextureSerializer.h"
#include "Project/Project.h"

#include <Fussion/Util/TextureImporter.h>

using namespace Fussion;

void TextureSerializer::Save(EditorAssetMetadata metadata, Ref<Asset> const& asset)
{
    (void)metadata;
    (void)asset;
}

Ref<Asset> TextureSerializer::Load(EditorAssetMetadata metadata)
{
    auto path = Project::assets_folder() / metadata.path;
    auto [data, width, height] = TextureImporter::load_image_from_file(path).value();

    auto texture_metadata = std::dynamic_pointer_cast<Texture2DMetadata>(metadata.custom_metadata);
    texture_metadata->width = CAST(s32, width);
    texture_metadata->height = CAST(s32, height);

    return Texture2D::create(data, *texture_metadata);
}
