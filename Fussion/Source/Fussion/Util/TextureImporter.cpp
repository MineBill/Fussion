#include "TextureImporter.h"

#include "stb_image_write.h"
#include "RHI/Device.h"
#include "Rendering/Renderer.h"

#include <Fussion/OS/FileSystem.h>
#include <Fussion/Util/stb_image.h>

namespace Fussion {
    auto TextureImporter::LoadImageFromMemory(std::span<u8> data) -> Maybe<Image>
    {
        Image image{};
        int actual_channels_on_image, w, h;

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
        Texture2DMetadata metadata{};
        metadata.Width = image->Width;
        metadata.Height = image->Height;
        return Texture2D::Create(image->Data, metadata);
    }

    auto TextureImporter::LoadTextureFromMemory(std::span<u8> data, bool is_normal_map) -> Maybe<Ref<Texture2D>>
    {
        auto image = LoadImageFromMemory(data);
        if (!image) {
            return None();
        }
        Texture2DMetadata metadata{};
        metadata.Width = image->Width;
        metadata.Height = image->Height;
        metadata.IsNormalMap = is_normal_map;
        return Texture2D::Create(image->Data, metadata);
    }

    void TextureImporter::SaveImageToFile(GPU::Texture const& texture, std::filesystem::path const& path)
    {
        (void)texture;
        (void)path;
        auto encoder = Renderer::Device().CreateCommandEncoder("Image Download Encoder");
        // ...
        Renderer::Device().SubmitCommandBuffer(encoder.Finish());
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
