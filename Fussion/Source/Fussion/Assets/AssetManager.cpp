#include "e5pch.h"
#include "AssetManager.h"

#include <utility>

namespace Fussion
{
    Ref<AssetManagerBase> AssetManager::s_Active;

    void AssetManager::SetActive(Ref<AssetManagerBase> manager)
    {
        s_Active = std::move(manager);
    }
}