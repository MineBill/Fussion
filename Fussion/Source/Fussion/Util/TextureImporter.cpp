#include "TextureImporter.h"

#include "Fussion/OS/FileSystem.h"
#include "Fussion/Rendering/Renderer.h"

#include "stb_image.h"
#include "stb_image_write.h"

namespace Fussion {
    auto TextureImporter::LoadImageFromMemory(std::span<u8> data) -> Maybe<Image>
    {
        Image image {};
        int actual_channels_on_image, w, h;
        stbi_set_flip_vertically_on_load(false);
        auto* d = stbi_load_from_memory(data.data(), CAST(int, data.size_bytes()), &w, &h, &actual_channels_on_image, 4);
        if (d == nullptr) {
            LOG_ERRORF("Failed to load image: {}", stbi_failure_reason());
            return None();
        }

        image.Data.resize(w * h * 4);
        std::copy_n(d, w * h * 4, image.Data.data());

        image.Width = CAST(u32, w);
        image.Height = CAST(u32, h);

        return image;
    }

    auto TextureImporter::LoadImageFromFile(std::filesystem::path const& path) -> Maybe<Image>
    {
        auto data = FileSystem::ReadEntireFileBinary(path);
        if (!data)
            return None();
        return LoadImageFromMemory(*data);
    }

    auto TextureImporter::LoadTextureFromFile(std::filesystem::path const& path) -> Maybe<Ref<Texture2D>>
    {
        auto image = LoadImageFromFile(path);
        if (!image) {
            return None();
        }
        Texture2DMetadata metadata {};
        metadata.Width = image->Width;
        metadata.Height = image->Height;
        metadata.Format = GPU::TextureFormat::RGBA8Unorm;
        return Texture2D::Create(image->Data, metadata);
    }

    auto TextureImporter::LoadTextureFromMemory(std::span<u8> data, bool is_normal_map) -> Maybe<Ref<Texture2D>>
    {
        auto image = LoadImageFromMemory(data);
        if (!image) {
            return None();
        }
        Texture2DMetadata metadata {};
        metadata.Width = image->Width;
        metadata.Height = image->Height;
        metadata.IsNormalMap = is_normal_map;
        return Texture2D::Create(image->Data, metadata);
    }

    void TextureImporter::SaveImageToFile(GPU::Texture const& texture, std::filesystem::path const& path)
    {
        (void)texture;
        (void)path;
        UNIMPLEMENTED;
    }

    auto TextureImporter::LoadHDRImageFromMemory(std::span<u8> data) -> Maybe<FloatImage>
    {
        FloatImage image {};
        int actual_channels_on_image, w, h;

        stbi_hdr_to_ldr_gamma(2.2f);
        stbi_hdr_to_ldr_scale(1.0f);
        stbi_set_flip_vertically_on_load(true);
        f32* image_data = stbi_loadf_from_memory(data.data(), CAST(int, data.size_bytes()), &w, &h, &actual_channels_on_image, 4);
        LOG_INFOF("Actual channels: {}", actual_channels_on_image);
        if (image_data == nullptr) {
            LOG_ERRORF("Failed to load image: {}", stbi_failure_reason());
            return None();
        }

        image.Data.resize(w * h * 4);
        std::copy_n(image_data, w * h * 4, image.Data.data());

        image.Width = CAST(u32, w);
        image.Height = CAST(u32, h);

        return image;
    }

    auto TextureImporter::LoadHDRImageFromFile(std::filesystem::path const& path) -> Maybe<FloatImage>
    {
        auto data = FileSystem::ReadEntireFileBinary(path);
        if (!data)
            return None();
        return LoadHDRImageFromMemory(*data);
    }

    auto TextureImporter::LoadHDRTextureFromMemory(std::span<u8> data, bool is_normal_map) -> Maybe<Ref<Texture2D>>
    {
        auto image = LoadHDRImageFromMemory(data);
        if (!image) {
            return None();
        }
        Texture2DMetadata metadata {};
        metadata.Width = image->Width;
        metadata.Height = image->Height;
        metadata.IsNormalMap = is_normal_map;
        metadata.Format = GPU::TextureFormat::RGBA16Float;
        return Texture2D::Create(image->Data, metadata);
    }
}
