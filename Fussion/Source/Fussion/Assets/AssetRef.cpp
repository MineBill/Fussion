#include "AssetRef.h"
#include "AssetManager.h"

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

    Asset* AssetRefBase::GetRaw(AssetType type) const
    {
        return AssetManager::GetAsset(m_Handle, type);
    }
}
