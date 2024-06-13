#include <utility>

#include "e5pch.h"
#include "AssetManagerBase.h"

namespace Engin5
{
    Ref<AssetManagerBase> AssetManager::s_Active;

    void AssetManager::SetActive(Ref<AssetManagerBase> manager)
    {
        s_Active = std::move(manager);
    }
}