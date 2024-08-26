#pragma once
#include "EditorAssetManager.h"
#include "Fussion/Core/Types.h"

#include <filesystem>

namespace fs = std::filesystem;

class Project {
public:
    static void Initialize();
    static void Save();
    static bool Load(fs::path const& path);

    static auto GetName() -> std::string_view { return s_ActiveProject->m_Name; }
    static auto GetAssetManager() -> Ref<EditorAssetManager>& { return s_ActiveProject->m_AssetManager; }
    static auto GetRoot() -> fs::path const& { return s_ActiveProject->m_ProjectPath; }
    static auto GetAssetRegistry() -> fs::path const& { return s_ActiveProject->m_AssetRegistryPath; }
    static auto GetAssetsFolder() -> fs::path const& { return s_ActiveProject->m_AssetsFolderPath; }
    static auto GetCacheFolder() -> fs::path const& { return s_ActiveProject->m_CacheFolderPath; }
    static auto GetScriptsFolder() -> fs::path const& { return s_ActiveProject->m_ScriptsFolderPath; }
    static auto GetLogsFolder() -> fs::path const& { return s_ActiveProject->m_LogsFolderPath; }

    /// Creates a new project at the specified location by creating all the necessary
    /// files and folders.
    /// @param path A path to an empty folder. If the path doesn't exist it will be created.
    /// @param name The name of the project. It <b>MUST</b> be path "friendly" because it will
    /// be used as the name of the project file.
    /// @returns A path the project file, which can used by Project::Load().
    static auto GenerateProject(fs::path const& path, std::string_view name) -> fs::path;

private:
    static Ptr<Project> s_ActiveProject;

    Ref<EditorAssetManager> m_AssetManager{};

    std::string m_Name{ "Unnamed Project" };

    fs::path m_ProjectPath;

    fs::path m_LogsFolderPath;
    fs::path m_AssetRegistryPath;
    fs::path m_AssetsFolderPath;
    fs::path m_ScriptsFolderPath;
    fs::path m_CacheFolderPath;
};
