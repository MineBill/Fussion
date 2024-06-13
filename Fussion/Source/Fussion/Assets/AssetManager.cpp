#include <utility>

#include "e5pch.h"
#include "AssetManagerBase.h"

namespace Fussion
{
    Ref<AssetManagerBase> AssetManager::s_Active;

    void AssetManager::SetActive(Ref<AssetManagerBase> manager)
    {
        s_Active = std::move(manager);
    }
}