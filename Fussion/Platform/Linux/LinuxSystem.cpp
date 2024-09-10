#include "Log/Log.h"
#include "OS/System.h"

#include <GLFW/glfw3.h>
// #define GLFW_EXPOSE_NATIVE_WAYLAND
// #include <GLFW/glfw3native.h>

namespace Fussion::System {
    bool prefers_dark()
    {
        return !prefers_light();
    }

    bool prefers_light()
    {
        return false;
    }

    auto get_known_folder(KnownFolders folder) -> std::filesystem::path
    {
        return {};
    }
    
    bool console_supports_color() {
        return false;
    }
}
