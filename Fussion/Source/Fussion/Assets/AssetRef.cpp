#include "AssetRef.h"
#include "AssetManager.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    AssetRefBase::AssetRefBase(AssetHandle handle): m_handle(handle)
    {
        if (!AssetManager::is_asset_handle_valid(m_handle)) {
            m_is_valid = false;
        }
    }

    bool AssetRefBase::is_loaded() const
    {
        if (!is_valid())
            return false;
        return AssetManager::is_asset_loaded(m_handle);
    }

    bool AssetRefBase::is_virtual() const
    {
        return AssetManager::is_asset_virtual(m_handle);
    }

    void AssetRefBase::set_handle(AssetHandle handle)
    {
        m_handle = handle;
        if (!AssetManager::is_asset_handle_valid(m_handle)) {
            m_is_valid = false;
        }
    }

    void AssetRefBase::wait_until_loaded() const
    {
        // Call once to trigger a load.
        (void)get_raw(type());
        // TODO: Fishy
        while (!is_loaded()) {}
    }

    void AssetRefBase::serialize(Serializer& ctx) const
    {
        if (is_virtual())
            return;
        FSN_SERIALIZE_MEMBER(m_handle);
    }

    void AssetRefBase::deserialize(Deserializer& ctx)
    {
        if (is_virtual())
            return;
        FSN_DESERIALIZE_MEMBER(m_handle);
    }

    Asset* AssetRefBase::get_raw(AssetType type) const
    {
        if (!AssetManager::is_asset_handle_valid(m_handle)) {
            return nullptr;
        }
        return AssetManager::get_asset(m_handle, type);
    }
}
