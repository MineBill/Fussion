#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/Core/Uuid.h"

#include <filesystem>

namespace Fussion {
    class AssetManagerBase {
    public:
        virtual ~AssetManagerBase() = default;

        [[nodiscard]]
        virtual auto IsAssetHandleValid(AssetHandle handle) const -> bool = 0;

        [[nodiscard]]
        virtual auto IsAssetLoaded(AssetHandle handle) -> bool = 0;

        [[nodiscard]]
        virtual auto GetAsset(AssetHandle handle, AssetType type) -> Asset* = 0;

        [[nodiscard]]
        virtual auto GetAsset(std::string const& path, AssetType type) -> Asset* = 0;

        [[nodiscard]]
        virtual auto CreateVirtualAsset(Ref<Asset> const& asset, std::string_view name = "(Unnamed) Virtual Asset", std::filesystem::path const& path = "") -> AssetHandle = 0;

        [[nodiscard]]
        virtual auto IsAssetVirtual(AssetHandle handle) -> bool = 0;

        [[nodiscard]]
        virtual auto GetAssetMetadata(AssetHandle handle) -> AssetMetadata* = 0;
    };
}

namespace Fsn = Fussion;
