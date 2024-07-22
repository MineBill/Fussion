#pragma once

#include "Project/EditorAssetManager.h"
#include "Fussion/Assets/Asset.h"

/// The AssetSerializer handles saving/loading files in the editor.
/// Implementations need to be able to save the asset in a text file
/// and then load it.
class AssetSerializer {
public:
    virtual ~AssetSerializer() = default;

    /// Save an asset file to disk. Usually in KDL.
    virtual void Save([[maybe_unused]] AssetMetadata metadata, [[maybe_unused]] Ref<Fussion::Asset> const& asset) {}

    /// Load a file from disk.
    /// @return The asset if load was successful, nullptr otherwise.
    virtual Ref<Fussion::Asset> Load(AssetMetadata metadata) = 0;
};
