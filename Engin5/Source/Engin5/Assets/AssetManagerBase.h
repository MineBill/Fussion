#pragma once
#include "Asset.h"
#include "Engin5/Core/UUID.h"

namespace Engin5
{
    class AssetManagerBase
    {
    public:
        virtual ~AssetManagerBase() = default;

        virtual bool IsAssetHandleValid(AssetHandle handle) = 0;
        virtual bool IsAssetLoaded(AssetHandle handle) = 0;

        virtual Asset* GetAsset(AssetHandle handle, AssetType type) = 0;
    };

    class AssetManager
    {
    public:
        static void SetActive(Ref<AssetManagerBase>);

        static bool IsAssetHandleValid(AssetHandle const handle)
        {
            return s_Active->IsAssetHandleValid(handle);
        }

        static bool IsAssetLoaded(AssetHandle handle)
        {
            return s_Active->IsAssetLoaded(handle);
        }

        static Asset* GetAsset(AssetHandle handle, AssetType type)
        {
            return s_Active->GetAsset(handle, type);
        }

        template<typename T>
        static T* GetAsset(AssetHandle handle)
        {
            return cast(T*, s_Active->GetAsset(handle, T::GetStaticType()));
        }
    private:
        static Ref<AssetManagerBase> s_Active;
    };

}