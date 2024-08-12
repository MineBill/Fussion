#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/Core/Uuid.h"

namespace Fussion {
    class AssetManagerBase {
    public:
        virtual ~AssetManagerBase() = default;

        [[nodiscard]]
        virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;

        [[nodiscard]]
        virtual bool IsAssetLoaded(AssetHandle handle) const = 0;

        [[nodiscard]]
        virtual Asset* GetAsset(AssetHandle handle, AssetType type) = 0;

        [[nodiscard]]
        virtual Asset* GetAsset(std::string const& path, AssetType type) = 0;

        virtual AssetHandle CreateVirtualAsset(Ref<Asset> const& asset, std::string_view name = "(Unnamed) Virtual Asset", std::filesystem::path const& path = "") = 0;

        virtual bool IsAssetVirtual(AssetHandle handle) = 0;

        virtual AssetMetadata* GetAssetMetadata(AssetHandle handle) = 0;
    };
}

namespace Fsn = Fussion;
