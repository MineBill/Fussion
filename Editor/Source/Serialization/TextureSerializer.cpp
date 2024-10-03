#include "EditorPCH.h"
#include "TextureSerializer.h"
#include "Project/Project.h"

#include <Fussion/Util/TextureImporter.h>

using namespace Fussion;

Ref<Asset> TextureSerializer::Load(EditorAssetMetadata metadata)
{
    auto path = Project::AssetsFolderPath() / metadata.Path;
    auto texture_metadata = std::dynamic_pointer_cast<Texture2DMetadata>(metadata.CustomMetadata);

    auto ext = path.extension();
    if (ext == ".hdr") {
        auto [data, width, height] = TextureImporter::LoadHDRImageFromFile(path).Unwrap();
        texture_metadata->Width = CAST(s32, width);
        texture_metadata->Height = CAST(s32, height);
        texture_metadata->Format = GPU::TextureFormat::RGBA32Float;
        texture_metadata->GenerateMipmaps = false;
        return Texture2D::Create(data, *texture_metadata);
    }

    auto [data, width, height] = TextureImporter::LoadImageFromFile(path).Unwrap();
    texture_metadata->Width = CAST(s32, width);
    texture_metadata->Height = CAST(s32, height);
    return Texture2D::Create(data, *texture_metadata);
}
