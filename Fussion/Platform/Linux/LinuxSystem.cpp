#include "Log/Log.h"
#include "OS/System.h"

#include <cstdlib>

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
        std::filesystem::path home = std::getenv("HOME");
        if (home.empty()) {
            LOG_WARNF("$HOME is not set");
        }

        std::filesystem::path path = "";
        switch (folder) {
        case KnownFolders::Downloads:
            path = home / "Downloads";
            break;
        case KnownFolders::Config:
            if (auto p = std::getenv("XDG_CONFIG_HOME")) {
                path = p;
            } else {
                path = home / ".config";
            }
            break;
        case KnownFolders::State:
            if (auto p = std::getenv("XDG_STATE_HOME")) {
                path = p;
            } else {
                path = home / ".local/state";
            }
            break;
        case KnownFolders::Data:
            if (auto p = std::getenv("XDG_DATA_HOME")) {
                path = p;
            } else {
                path = home / ".local/share";
            }
            break;
        case KnownFolders::Temp:
            // name = "XDG_" break;
            path = "/tmp";
            break;
        }
        return path;
    }

    bool ConsoleSupportsColor()
    {
        return false;
    }
}
