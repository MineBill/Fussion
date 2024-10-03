#include "FussionPCH.h"
#include "AssetManager.h"

#include <utility>

namespace Fussion {
    AssetManagerBase* AssetManager::s_Active;

    void AssetManager::SetActive(AssetManagerBase* manager)
    {
        s_Active = manager;
    }
}
