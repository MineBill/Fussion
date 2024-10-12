#include "Log/Log.h"
#include "OS/System.h"

namespace Fussion::System {
    bool PrefersDark()
    {
        return !PrefersLight();
    }

    bool PrefersLight()
    {
        return false;
    }

    auto GetKnownFolder(KnownFolders folder) -> std::filesystem::path
    {
        (void)folder;
        return {};
    }

    bool ConsoleSupportsColor()
    {
        return false;
    }
}
