#include "AssetRef.h"
#include "AssetManager.h"

namespace Fussion {
    bool AssetRefBase::IsVirtual() const
    {
        return AssetManager::IsAssetVirtual(m_Handle);
    }

    Asset* AssetRefBase::GetRaw(AssetType type) const
    {
        return AssetManager::GetAsset(m_Handle, type);
    }
}
