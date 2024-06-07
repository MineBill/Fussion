#pragma once
#include "Asset.h"
#include "Engin5/Core/UUID.h"

namespace Engin5
{
    class AssetManager
    {
    public:
        virtual ~AssetManager() = default;

        virtual bool IsAssetHandleValid(AssetHandle handle) = 0;
        virtual bool IsAssetLoaded(AssetHandle handle) = 0;

        virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
    };
}