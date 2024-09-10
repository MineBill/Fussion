#pragma once

namespace Fussion::System {
    /// Returns if the OS is currently in dark mode.
    bool prefers_dark();

    /// Returns if the OS is currently in light mode.
    bool prefers_light();

    enum class KnownFolders {
        Downloads,
        // ...
        /// Folder for application specific files
        /// Usually %APPDATA% on windows, $XDG_CONFIG_HOME on linux.
        AppData,
    };

    /// Returns the location of the specified known folder for the current OS.
    auto get_known_folder(KnownFolders folder) -> std::filesystem::path;
    
    bool console_supports_color();
}
