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

    Fsn::AssetType type = Fsn::AssetType::Invalid;
    std::filesystem::path path;
    std::string name;
    bool is_virtual = false;
    bool dont_serialize = false;

    /// Runtime-only flag to detect if the asset has been modified.
    bool dirty = false;

    AssetLoadState load_state{ AssetLoadState::Unloaded };

    Fussion::AssetHandle handle;

    // TODO: Investigate if using Ref is a good idea.
    Ref<Fussion::AssetMetadata> custom_metadata{ nullptr };
    bool is_valid() const { return type != Fsn::AssetType::Invalid; }
};