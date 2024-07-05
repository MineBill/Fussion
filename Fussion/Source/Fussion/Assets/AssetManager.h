#pragma once
#include "Fussion/Assets/AssetManagerBase.h"
#include "Fussion/Assets/AssetRef.h"

namespace Fussion
{
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
        static AssetRef<T> GetAsset(AssetHandle handle)
        {
            return AssetRef<T>(handle);
        }
    private:
        static Ref<AssetManagerBase> s_Active;
    };
}

