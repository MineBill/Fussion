#include "AssetRef.h"
#include "AssetManager.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    bool AssetRefBase::IsLoaded() const
    {
        return AssetManager::IsAssetLoaded(m_Handle);
    }

    bool AssetRefBase::IsVirtual() const
    {
        return AssetManager::IsAssetVirtual(m_Handle);
    }

    void AssetRefBase::WaitUntilLoaded() const
    {
        // Call once to trigger a load.
        (void)GetRaw(GetType());
        // TODO: Fishy
        while (!IsLoaded()) {}
    }

    void AssetRefBase::Serialize(Serializer& ctx) const
    {
        FSN_SERIALIZE_MEMBER(m_Handle);
    }

    void AssetRefBase::Deserialize(Deserializer& ctx)
    {
        FSN_DESERIALIZE_MEMBER(m_Handle);
    }

    Asset* AssetRefBase::GetRaw(AssetType type) const
    {
        return AssetManager::GetAsset(m_Handle, type);
    }
}
