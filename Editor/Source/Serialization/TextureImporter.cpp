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
        auto [data, width, height] = TextureLoader::LoadHDRImageFromFile(path).Unwrap();
        texture_metadata.Width = CAST(s32, width);
        texture_metadata.Height = CAST(s32, height);
        texture_metadata.Format = GPU::TextureFormat::RGBA32Float;
        texture_metadata.GenerateMipmaps = false;
        return Texture2D::Create(data, texture_metadata);
    }

    auto [data, width, height] = TextureLoader::LoadImageFromFile(path).Unwrap();
    texture_metadata.Width = CAST(s32, width);
    texture_metadata.Height = CAST(s32, height);
    return Texture2D::Create(data, texture_metadata);
}
