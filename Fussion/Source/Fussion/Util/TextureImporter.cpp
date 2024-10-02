#include "TextureImporter.h"

#include "stb_image_write.h"
#include "Rendering/Renderer.h"

#include <Fussion/OS/FileSystem.h>
#include <Fussion/Util/stb_image.h>

namespace Fussion {
    auto TextureImporter::load_image_from_memory(std::span<u8> data) -> Maybe<Image>
    {
        Image image{};
        int actual_channels_on_image, w, h;
        stbi_set_flip_vertically_on_load(false);
        auto* d = stbi_load_from_memory(data.data(), CAST(int, data.size_bytes()), &w, &h, &actual_channels_on_image, 4);
        if (d == nullptr) {
            LOG_ERRORF("Failed to load image: {}", stbi_failure_reason());
            return None();
        }

        image.data.resize(w * h * 4);
        std::copy_n(d, w * h * 4, image.data.data());

        image.width = CAST(u32, w);
        image.height = CAST(u32, h);

        return image;
    }

    auto TextureImporter::load_image_from_file(std::filesystem::path const& path) -> Maybe<Image>
    {
        auto data = FileSystem::read_entire_file_binary(path);
        if (!data)
            return None();
        return load_image_from_memory(*data);
    }

    auto TextureImporter::load_texture_from_file(std::filesystem::path const& path) -> Maybe<Ref<Texture2D>>
    {
        auto image = load_image_from_file(path);
        if (!image) {
            return None();
        }
        Texture2DMetadata metadata{};
        metadata.width = image->width;
        metadata.height = image->height;
        metadata.format = GPU::TextureFormat::RGBA8Unorm;
        return Texture2D::create(image->data, metadata);
    }

    auto TextureImporter::load_texture_from_memory(std::span<u8> data, bool is_normal_map) -> Maybe<Ref<Texture2D>>
    {
        auto image = load_image_from_memory(data);
        if (!image) {
            return None();
        }
        Texture2DMetadata metadata{};
        metadata.width = image->width;
        metadata.height = image->height;
        metadata.is_normal_map = is_normal_map;
        return Texture2D::create(image->data, metadata);
    }

    void TextureImporter::save_image_to_file(GPU::Texture const& texture, std::filesystem::path const& path)
    {
        (void)texture;
        (void)path;
        UNIMPLEMENTED;
    }

    auto TextureImporter::load_hdr_image_from_memory(std::span<u8> data) -> Maybe<FloatImage>
    {
        FloatImage image{};
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

        image.data.resize(w * h * 4);
        std::copy_n(image_data, w * h * 4, image.data.data());

        image.width = CAST(u32, w);
        image.height = CAST(u32, h);

        return image;
    }

    auto TextureImporter::load_hdr_image_from_file(std::filesystem::path const& path) -> Maybe<FloatImage>
    {
        auto data = FileSystem::read_entire_file_binary(path);
        if (!data)
            return None();
        return load_hdr_image_from_memory(*data);
    }

    auto TextureImporter::load_hdr_texture_from_memory(std::span<u8> data, bool is_normal_map) -> Maybe<Ref<Texture2D>>
    {
        auto image = load_hdr_image_from_memory(data);
        if (!image) {
            return None();
        }
        Texture2DMetadata metadata{};
        metadata.width = image->width;
        metadata.height = image->height;
        metadata.is_normal_map = is_normal_map;
        metadata.format = GPU::TextureFormat::RGBA16Float;
        return Texture2D::create(image->data, metadata);
    }
}
