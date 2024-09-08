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
        auto encoder = Renderer::device().create_command_encoder("Image Download Encoder");
        // ...
        Renderer::device().submit_command_buffer(encoder.finish());
        // auto& image_spec = image->GetSpec();
        //
        // auto buffer_spec = RHI::BufferSpecification{
        //     .Label = "Save Image To File",
        //     .Usage = RHI::BufferUsage::TransferDestination,
        //     .Size = image_spec.Width * image_spec.Height * 4,
        //     .Mapped = true
        // };
        // auto buffer = RHI::Device::Instance()->CreateBuffer(buffer_spec);
        //
        // auto old_layout = image->GetSpec().FinalLayout;
        // image->TransitionLayout(RHI::ImageLayout::TransferSrcOptimal);
        // auto cmd = RHI::Device::Instance()->BeginSingleTimeCommand();
        // cmd->CopyImageToBuffer(image, buffer, Vector2(image_spec.Width, image_spec.Height));
        // RHI::Device::Instance()->EndSingleTimeCommand(cmd);
        // image->TransitionLayout(old_layout);
        //
        // buffer->GetMappedData();
        // stbi_write_jpg(path.string().data(), CAST(s32, image_spec.Width), CAST(s32, image_spec.Height), 4, buffer->GetMappedData(), 100);
    }
}
