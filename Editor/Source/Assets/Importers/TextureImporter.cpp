#include "TextureImporter.h"

#include "Fussion/OS/FileSystem.h"
#include "Fussion/Util/stb_image.h"

using namespace Fussion;

auto TextureImporter::LoadImageFromMemory(std::span<u8> data) -> Image
{
    Image image{};
    int c, w, h;

    auto* d = stbi_load_from_memory(data.data(), CAST(int, data.size_bytes()), &w, &h, &c, 4);
    if (d == nullptr) {
        LOG_ERRORF("Failed to load image: {}", stbi_failure_reason());
        return {};
    }

    image.Data.resize(w * h * c);
    std::copy_n(d, w * h * c, image.Data.data());

    // image.Data = std::vector<u8>{ d, CAST(std::size_t, w * h * c) };
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
    return Texture2D::Create(Data, { .Width = CAST(s32, Width), .Height = CAST(s32, Height) });
}
