#pragma once

#include <filesystem>

namespace Fussion {
    class Application;
    class System final {
        friend Application;

    public:
        enum class SystemType {
            Invalid,
            Windows,
            Linux,
        };

        enum class WindowingSystem {
            Invalid,
            Windows,
            X11,
            Wayland,
        };

        enum class Desktop {
            Invalid,
            Windows,
            KDE,
            Gnome,
        };

        static SystemType GetCurrentSystemType();

        struct Info {
            SystemType Type {};
            WindowingSystem WindowingSystem {};
            Desktop Desktop {};
        };

        static Info const& GetSystemInfo();

        /// Returns if the OS is currently in dark mode.
        static bool PrefersDark();

        /// Returns if the OS is currently in light mode.
        static bool PrefersLight();

        enum class KnownFolders {
            Downloads,

            /// Should be used for configuration data.
            Config,
            /// Defines the base directory relative to which user-specific data files should be stored.
            Data,
            /// Contains state data that should persist between (application) restarts, but that is not important or portable enough to the user that it should be stored in KnownFolders::Data
            State,
            Temp,

            // ...
            /// Folder for application specific files
            /// Usually %APPDATA% on windows, $XDG_CONFIG_HOME on linux.
            AppData = Data,
        };

        /// Returns the location of the specified known folder for the current OS.
        static auto GetKnownFolder(KnownFolders folder) -> std::filesystem::path;

        static bool ConsoleSupportsColor();

    private:
        static void Initialize();
    };
}
