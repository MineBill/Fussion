#include "EditorAssetManager.h"

Ref<Engin5::Asset> EditorAssetManager::GetAsset(Engin5::AssetHandle handle)
{
    return m_LoadedAssets[handle];
}

bool EditorAssetManager::IsAssetLoaded(Engin5::AssetHandle handle)
{
    return m_Registry.contains(handle);
}

bool EditorAssetManager::IsAssetHandleValid(Engin5::AssetHandle handle)
{
    return m_LoadedAssets.contains(handle);
}
