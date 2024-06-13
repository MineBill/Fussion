#pragma once

#include "Project/EditorAssetManager.h"
#include "Engin5/Assets/Asset.h"

/// The AssetSerializer handles saving/loading files in the editor.
/// Implementations need to be able to save the asset in a text file
/// and then load it.
class AssetSerializer
{
public:
    virtual ~AssetSerializer() = default;

    /// Save an asset file to disk. Usually in KDL.
    virtual void Save(AssetMetadata metadata, Engin5::Asset* asset) = 0;

    /// Load a file from disk.
    /// @return The asset if load was successful, nullptr otherwise.
    virtual Engin5::Asset* Load(AssetMetadata metadata) = 0;
};