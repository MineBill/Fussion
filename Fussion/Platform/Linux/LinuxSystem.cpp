#include "Log/Log.h"
#include "OS/System.h"

#include <GLFW/glfw3.h>
// #define GLFW_EXPOSE_NATIVE_WAYLAND
// #include <GLFW/glfw3native.h>

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
