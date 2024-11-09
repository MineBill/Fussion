#include "Log/Log.h"
#include "OS/System.h"

#include <cstdlib>

namespace Fussion {
    struct Data {
        System::Info info {};
    };

    static Data g_data;

    void System::initialize()
    {
        if (auto xdg_current_desktop = std::getenv("XDG_CURRENT_DESKTOP")) {
            auto desktop = std::string(xdg_current_desktop);
            if (desktop == "KDE") {
                g_data.info.desktop = Desktop::KDE;
            } else if (desktop == "GNOME") {
                g_data.info.desktop = Desktop::Gnome;
            }
        } else {
            LOG_WARNF("XDG_CURRENT_DESKTOP not set, cannot determine current desktop environment");
        }

        if (std::getenv("FSN_LINUX_X11")) {
            g_data.info.windowing_system = WindowingSystem::X11;
        } else {
            if (auto xdg_session_type = std::getenv("XDG_SESSION_TYPE"); xdg_session_type) {
                auto session = std::string(xdg_session_type);
                if (session == "wayland") {
                    g_data.info.windowing_system = WindowingSystem::Wayland;
                } else if (session == "x11") {
                    g_data.info.windowing_system = WindowingSystem::X11;
                }
            }
        }
    }

    System::Info const& System::system_info()
    {
        return g_data.info;
    }

    bool System::prefers_dark()
    {
        return !prefers_light();
    }

    bool System::prefers_light()
    {
        return false;
    }

    auto System::get_known_folder(KnownFolders folder) -> std::filesystem::path
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

    bool System::does_console_support_color()
    {
        return false;
    }
}
