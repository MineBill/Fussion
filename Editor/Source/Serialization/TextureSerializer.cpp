#include "TextureSerializer.h"
#include "Project/Project.h"

#include <Fussion/Util/TextureImporter.h>

using namespace Fussion;

void TextureSerializer::Save(AssetMetadata metadata, Ref<Asset> const& asset)
{
    (void)metadata;
    (void)asset;
}

Ref<Asset> TextureSerializer::Load(AssetMetadata metadata)
{
    auto path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    return TextureImporter::LoadTextureFromFile(path);
}
