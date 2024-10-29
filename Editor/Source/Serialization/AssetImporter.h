#pragma once

#include "Project/EditorAssetMetadata.h"

#include <Fussion/Assets/Asset.h>

/// An AssetImporter loads an asset from a source format into memory.
class AssetImporter {
public:
    virtual ~AssetImporter() = default;

    /// Load a file from disk.
    /// @return The asset if load was successful, nullptr otherwise.
    virtual auto Import(std::filesystem::path const& path) -> Ref<Fussion::Asset> = 0;
};
