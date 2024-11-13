#pragma once
#include "AssetImporter.h"

class TextureImporter final : public AssetImporter {
public:
    virtual auto import(std::filesystem::path const& path) -> Ref<Fussion::BinaryAsset> override;
};
