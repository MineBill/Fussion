#include "FussionPCH.h"
#include "AssetManager.h"

#include <utility>

namespace Fussion
{
    AssetManagerBase* AssetManager::s_active;

    void AssetManager::set_active(AssetManagerBase* manager)
    {
        s_active = manager;
    }
}