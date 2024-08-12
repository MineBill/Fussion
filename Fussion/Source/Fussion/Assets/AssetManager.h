#pragma once
#include "Fussion/Assets/AssetManagerBase.h"
#include "Fussion/Assets/AssetRef.h"

namespace Fussion {
class AssetManager {
public:
    static void SetActive(Ref<AssetManagerBase>);

    static AssetHandle CreateVirtualAsset(Ref<Asset> const& asset, std::string_view name = "(Unnamed) Virtual Asset")
    {
        return s_Active->CreateVirtualAsset(asset, name);
    }

    template<std::derived_from<Asset> T>
    static AssetRef<T> CreateVirtualAssetRef(Ref<Asset> const& asset, std::string_view name = "(Unnamed) Virtual Asset")
    {
        return AssetRef<T>(s_Active->CreateVirtualAsset(asset, name));
    }

    static AssetHandle CreateVirtualAssetWithPath(Ref<Asset> const& asset, std::filesystem::path const& path, std::string_view name = "(Unnamed) Virtual Asset")
    {
        return s_Active->CreateVirtualAsset(asset, name, path);
    }

    template<std::derived_from<Asset> T>
    static AssetRef<T> CreateVirtualAssetRefWithPath(Ref<Asset> const& asset, std::filesystem::path const& path, std::string_view name = "(Unnamed) Virtual Asset")
    {
        return AssetRef<T>(s_Active->CreateVirtualAsset(asset, name, path));
    }

    static bool IsAssetHandleValid(AssetHandle const handle)
    {
        return s_Active->IsAssetHandleValid(handle);
    }

    static bool IsAssetLoaded(AssetHandle handle)
    {
        return s_Active->IsAssetLoaded(handle);
    }

    static bool IsAssetVirtual(AssetHandle handle)
    {
        return s_Active->IsAssetVirtual(handle);
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

    template<typename T>
    static T* GetAsset(std::string const& path)
    {
        return CAST(T*, s_Active->GetAsset(path, T::GetStaticType()));
    }

    template<typename T>
    static auto GetAssetMetadata(AssetHandle handle) -> T*
    {
        auto ptr = s_Active->GetAssetMetadata(handle);
        return dynamic_cast<T*>(ptr);
    }

private:
    static Ref<AssetManagerBase> s_Active;
};
}
