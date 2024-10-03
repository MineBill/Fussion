#include "AssetRef.h"

#include "AssetManager.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    AssetRefBase::AssetRefBase(AssetHandle handle)
        : m_Handle(handle)
    {
        if (!AssetManager::IsAssetHandleValid(m_Handle)) {
            m_IsValid = false;
        }
    }

    bool AssetRefBase::IsLoaded() const
    {
        if (!IsValid())
            return false;
        return AssetManager::IsAssetLoaded(m_Handle);
    }

    bool AssetRefBase::IsVirtual() const
    {
        return AssetManager::IsAssetVirtual(m_Handle);
    }

    void AssetRefBase::SetHandle(AssetHandle handle)
    {
        m_Handle = handle;
        if (!AssetManager::IsAssetHandleValid(m_Handle)) {
            m_IsValid = false;
        }
    }

    void AssetRefBase::WaitUntilLoaded() const
    {
        // Call once to trigger a load.
        (void)GetRaw(GetType());
        // TODO: Fishy
        while (!IsLoaded()) { }
    }

    void AssetRefBase::Serialize(Serializer& ctx) const
    {
        ctx.Write("handle", IsVirtual() ? 0_u64 : CAST(u64, this->m_Handle));
    }

    void AssetRefBase::Deserialize(Deserializer& ctx)
    {
        if (IsVirtual())
            return;
        FSN_DESERIALIZE_MEMBER(m_Handle);
    }

    Asset* AssetRefBase::GetRaw(AssetType type) const
    {
        if (!AssetManager::IsAssetHandleValid(m_Handle)) {
            return nullptr;
        }
        return AssetManager::GetAsset(m_Handle, type);
    }
}
