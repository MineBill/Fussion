#pragma once
#include "AssetImporter.h"

class TextureImporter final : public AssetImporter {
public:
    virtual auto Import(std::filesystem::path const& path) -> Ref<Fussion::Asset> override;
};
