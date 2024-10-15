#pragma once

#include <filesystem>

namespace Fussion::System {
    /// Returns if the OS is currently in dark mode.
    bool PrefersDark();

    /// Returns if the OS is currently in light mode.
    bool PrefersLight();

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
    auto GetKnownFolder(KnownFolders folder) -> std::filesystem::path;

    bool ConsoleSupportsColor();
}
