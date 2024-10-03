#pragma once

namespace Fussion::System {
    /// Returns if the OS is currently in dark mode.
    bool PrefersDark();

    /// Returns if the OS is currently in light mode.
    bool PrefersLight();

    enum class KnownFolders {
        Downloads,
        // ...
        /// Folder for application specific files
        /// Usually %APPDATA% on windows, $XDG_CONFIG_HOME on linux.
        AppData,
    };

    /// Returns the location of the specified known folder for the current OS.
    auto GetKnownFolder(KnownFolders folder) -> std::filesystem::path;

    bool ConsoleSupportsColor();
}
