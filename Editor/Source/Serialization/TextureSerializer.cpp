#include "EditorPCH.h"
#include "TextureSerializer.h"
#include "Project/Project.h"

#include <Fussion/Assets/AssetManager.h>
#include <Fussion/Util/TextureImporter.h>

using namespace Fussion;

void TextureSerializer::Save(EditorAssetMetadata metadata, Ref<Asset> const& asset)
{
    (void)metadata;
    (void)asset;
}

Ref<Asset> TextureSerializer::Load(EditorAssetMetadata metadata)
{
    auto path = Project::ActiveProject()->GetAssetsFolder() / metadata.Path;
    auto [data, width, height] = TextureImporter::LoadImageFromFile(path);

    auto texture_metadata = std::dynamic_pointer_cast<Texture2DMetadata>(metadata.CustomMetadata);
    texture_metadata->Width = width;
    texture_metadata->Height = height;

    return Texture2D::Create(data, *texture_metadata);
}
