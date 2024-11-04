#include "Log/Log.h"
#include "OS/System.h"

#include <cstdlib>

namespace Fussion {
    struct Data {
        System::Info Info {};
    };

    static Data g_Data;

    void System::Initialize()
    {
        if (auto cstr = std::getenv("XDG_CURRENT_DESKTOP")) {
            auto desktop = std::string(cstr);
            if (desktop == "KDE") {
                g_Data.Info.Desktop = Desktop::KDE;
            } else if (desktop == "GNOME") {
                g_Data.Info.Desktop = Desktop::Gnome;
            }
        } else {
            LOG_WARNF("XDG_CURRENT_DESKTOP not set, cannot determine current desktop environment");
        }

        if (auto cstr = std::getenv("FSN_LINUX_X11")) {
            g_Data.Info.WindowingSystem = WindowingSystem::X11;
        } else {
            if (cstr = std::getenv("XDG_SESSION_TYPE"); cstr) {
                auto session = std::string(cstr);
                if (session == "wayland") {
                    g_Data.Info.WindowingSystem = WindowingSystem::Wayland;
                } else if (session == "x11") {
                    g_Data.Info.WindowingSystem = WindowingSystem::X11;
                }
            }
        }
    }

    System::Info const& System::GetSystemInfo()
    {
        return g_Data.Info;
    }

    bool System::PrefersDark()
    {
        return !PrefersLight();
    }

    bool System::PrefersLight()
    {
        return false;
    }

    auto System::GetKnownFolder(KnownFolders folder) -> std::filesystem::path
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

    bool System::ConsoleSupportsColor()
    {
        return false;
    }

}
