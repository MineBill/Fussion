#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/Core/UUID.h"

namespace Fussion
{
    class AssetManagerBase
    {
    public:
        virtual ~AssetManagerBase() = default;

        virtual bool IsAssetHandleValid(AssetHandle handle) = 0;
        virtual bool IsAssetLoaded(AssetHandle handle) = 0;

        virtual Asset* GetAsset(AssetHandle handle, AssetType type) = 0;
    };
}

namespace Fsn = Fussion;