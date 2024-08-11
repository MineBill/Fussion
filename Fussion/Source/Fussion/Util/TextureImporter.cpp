#include "TextureImporter.h"

#include "stb_image_write.h"
#include "RHI/Device.h"

#include <Fussion/OS/FileSystem.h>
#include <Fussion/Util/stb_image.h>

namespace Fussion {
    auto TextureImporter::LoadImageFromMemory(std::span<u8> data) -> Image
    {
        Image image{};
        int actual_channels_on_image, w, h;

        auto* d = stbi_load_from_memory(data.data(), CAST(int, data.size_bytes()), &w, &h, &actual_channels_on_image, 4);
        if (d == nullptr) {
            LOG_ERRORF("Failed to load image: {}", stbi_failure_reason());
            return {};
        }

        image.Data.resize(w * h * 4);
        std::copy_n(d, w * h * 4, image.Data.data());

        image.Width = CAST(u32, w);
        image.Height = CAST(u32, h);

        return image;
    }

    auto TextureImporter::LoadImageFromFile(std::filesystem::path const& path) -> Image
    {
        auto data = FileSystem::ReadEntireFileBinary(path);
        return LoadImageFromMemory(data);
    }

    Ref<Texture2D> TextureImporter::LoadTextureFromFile(std::filesystem::path const& path)
    {
        auto [Data, Width, Height] = LoadImageFromFile(path);
        Texture2DMetadata metadata{};
        metadata.Width = Width;
        metadata.Height = Height;
        return Texture2D::Create(Data, metadata);
    }

    auto TextureImporter::LoadTextureFromMemory(std::span<u8> data, bool is_normal_map) -> Ref<Texture2D>
    {
        auto [Data, Width, Height] = LoadImageFromMemory(data);
        Texture2DMetadata metadata{};
        metadata.Width = Width;
        metadata.Height = Height;
        metadata.IsNormalMap = is_normal_map;
        return Texture2D::Create(Data, metadata);
    }

    void TextureImporter::SaveImageToFile(Ref<RHI::Image> const& image, std::filesystem::path const& path)
    {
        auto& image_spec = image->GetSpec();

        auto buffer_spec = RHI::BufferSpecification{
            .Label = "Save Image To File",
            .Usage = RHI::BufferUsage::TransferDestination,
            .Size = image_spec.Width * image_spec.Height * 4,
            .Mapped = true
        };
        auto buffer = RHI::Device::Instance()->CreateBuffer(buffer_spec);

        auto old_layout = image->GetSpec().FinalLayout;
        image->TransitionLayout(RHI::ImageLayout::TransferSrcOptimal);
        auto cmd = RHI::Device::Instance()->BeginSingleTimeCommand();
        cmd->CopyImageToBuffer(image, buffer, Vector2(image_spec.Width, image_spec.Height));
        RHI::Device::Instance()->EndSingleTimeCommand(cmd);
        image->TransitionLayout(old_layout);

        buffer->GetMappedData();
        stbi_write_jpg(path.string().data(), CAST(s32, image_spec.Width), CAST(s32, image_spec.Height), 4, buffer->GetMappedData(), 100);
    }
}
