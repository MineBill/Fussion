#pragma once
#include "Fussion/Assets/AssetManagerBase.h"
#include "Fussion/Assets/AssetRef.h"

namespace Fussion {
class AssetManager {
public:
    static void set_active(AssetManagerBase*);

    static AssetHandle create_virtual_asset(Ref<Asset> const& asset, std::string_view name = "(Unnamed) Virtual Asset")
    {
        return s_active->create_virtual_asset(asset, name);
    }

    template<std::derived_from<Asset> T>
    static AssetRef<T> create_virtual_asset_ref(Ref<Asset> const& asset, std::string_view name = "(Unnamed) Virtual Asset")
    {
        return AssetRef<T>(s_active->create_virtual_asset(asset, name));
    }

    static AssetHandle create_virtual_asset_with_path(Ref<Asset> const& asset, std::filesystem::path const& path, std::string_view name = "(Unnamed) Virtual Asset")
    {
        return s_active->create_virtual_asset(asset, name, path);
    }

    template<std::derived_from<Asset> T>
    static AssetRef<T> create_virtual_asset_ref_with_path(Ref<Asset> const& asset, std::filesystem::path const& path, std::string_view name = "(Unnamed) Virtual Asset")
    {
        return AssetRef<T>(s_active->create_virtual_asset(asset, name, path));
    }

    static bool is_asset_handle_valid(AssetHandle const handle)
    {
        return s_active->is_asset_handle_valid(handle);
    }

    static bool is_asset_loaded(AssetHandle handle)
    {
        return s_active->is_asset_loaded(handle);
    }

    static bool is_asset_virtual(AssetHandle handle)
    {
        return s_active->is_asset_virtual(handle);
    }

    static Asset* get_asset(AssetHandle handle, AssetType type)
    {
        return s_active->get_asset(handle, type);
    }

    template<typename T>
    static AssetRef<T> get_asset(AssetHandle handle)
    {
        // Trigger an asset load.
        get_asset(handle, T::static_type());
        return AssetRef<T>(handle);
    }

    template<typename T>
    static T* get_asset(std::string const& path)
    {
        return CAST(T*, s_active->get_asset(path, T::static_type()));
    }

    template<typename T>
    static auto get_asset_metadata(AssetHandle handle) -> T*
    {
        auto ptr = s_active->get_asset_metadata(handle);
        return dynamic_cast<T*>(ptr);
    }

private:
    static AssetManagerBase* s_active;
};
}
