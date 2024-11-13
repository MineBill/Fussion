#pragma once
#include "AssetImporter.h"

class MeshImporter final : public AssetImporter {
public:
    virtual auto import(std::filesystem::path const& path) -> Ref<Fussion::BinaryAsset> override;
};
