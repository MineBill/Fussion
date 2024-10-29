#pragma once
#include "AssetImporter.h"

class MeshImporter final : public AssetImporter {
public:
    virtual auto Import(std::filesystem::path const& path) -> Ref<Fussion::Asset> override;
};
