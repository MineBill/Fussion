#include "AssetRef.h"
#include "AssetManager.h"

Fussion::Asset* Fussion::AssetRefBase::GetRaw(AssetType type) const
{
    return AssetManager::GetAsset(m_Handle, type);
}
