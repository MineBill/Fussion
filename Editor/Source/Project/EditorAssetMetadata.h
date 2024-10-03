#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/Core/Types.h"

#include <filesystem>
#include <string>

enum class AssetLoadState {
    Unloaded = 0,
    Loading,
    Loaded,
    FailedToLoad,
};

struct EditorAssetMetadata final {
    Fsn::AssetType Type = Fsn::AssetType::Invalid;
    std::filesystem::path Path;
    std::string Name;
    bool IsVirtual = false;
    bool DontSerialize = false;

    /// Runtime-only flag to detect if the asset has been modified.
    bool Dirty = false;

    AssetLoadState LoadState { AssetLoadState::Unloaded };

    Fussion::AssetHandle Handle;

    // TODO: Investigate if using Ref is a good idea.
    Ref<Fussion::AssetMetadata> CustomMetadata { nullptr };
    bool IsValid() const { return Type != Fsn::AssetType::Invalid; }
};
