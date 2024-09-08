#include "AssetRef.h"
#include "AssetManager.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    bool AssetRefBase::is_loaded() const
    {
        return AssetManager::is_asset_loaded(m_handle);
    }

    bool AssetRefBase::is_virtual() const
    {
        return AssetManager::is_asset_virtual(m_handle);
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
        FSN_SERIALIZE_MEMBER(m_handle);
    }

    void AssetRefBase::deserialize(Deserializer& ctx)
    {
        FSN_DESERIALIZE_MEMBER(m_handle);
    }

    Asset* AssetRefBase::get_raw(AssetType type) const
    {
        return AssetManager::get_asset(m_handle, type);
    }
}
