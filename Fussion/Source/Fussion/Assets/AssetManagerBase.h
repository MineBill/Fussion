#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/Core/Uuid.h"

#include <filesystem>

namespace Fussion {
    class AssetManagerBase {
    public:
        virtual ~AssetManagerBase() = default;

        [[nodiscard]]
        virtual auto is_asset_handle_valid(AssetHandle handle) const -> bool = 0;

        [[nodiscard]]
        virtual auto is_asset_loaded(AssetHandle handle) -> bool = 0;

        [[nodiscard]]
        virtual auto get_asset(AssetHandle handle, AssetType type) -> Asset* = 0;

        [[nodiscard]]
        virtual auto get_asset(std::string const& path, AssetType type) -> Asset* = 0;

        [[nodiscard]]
        virtual auto create_virtual_asset(Ref<Asset> const& asset, std::string_view name = "(Unnamed) Virtual Asset", std::filesystem::path const& path = "") -> AssetHandle = 0;

        [[nodiscard]]
        virtual auto is_asset_virtual(AssetHandle handle) -> bool = 0;

        [[nodiscard]]
        virtual auto get_asset_metadata(AssetHandle handle) -> AssetMetadata* = 0;
    };
}

namespace Fsn = Fussion;
