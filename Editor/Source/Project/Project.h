#pragma once
#include <filesystem>

#include "EditorAssetManager.h"
#include "Fussion/Core/Types.h"

class Project {
public:
    static Ref<Project> ActiveProject()
    {
        return s_ActiveProject;
    }

    static void Initialize();
    static void Save(std::filesystem::path path = {});
    static bool Load(std::filesystem::path const& path);

    static auto GetAssetManager() -> Ref<EditorAssetManager>& { return s_ActiveProject->m_AssetManager; }
    static auto GetRoot() -> std::filesystem::path const& { return s_ActiveProject->m_ProjectPath; }
    static auto GetAssetRegistry() -> std::filesystem::path const& { return s_ActiveProject->m_AssetRegistryPath; }
    static auto GetAssetsFolder() -> std::filesystem::path const& { return s_ActiveProject->m_AssetsFolderPath; }
    static auto GetCacheFolder() -> std::filesystem::path const& { return s_ActiveProject->m_CacheFolderPath; }
    static auto GetScriptsFolder() -> std::filesystem::path const& { return s_ActiveProject->m_ScriptsFolderPath; }
    static auto GetLogsFolder() -> std::filesystem::path const& { return s_ActiveProject->m_LogsFolderPath; }

private:
    static Ref<Project> s_ActiveProject;

    Ref<EditorAssetManager> m_AssetManager{};

    std::string m_Name;

    std::filesystem::path m_ProjectPath;

    std::filesystem::path m_LogsFolderPath;
    std::filesystem::path m_AssetRegistryPath;
    std::filesystem::path m_AssetsFolderPath;
    std::filesystem::path m_ScriptsFolderPath;
    std::filesystem::path m_CacheFolderPath;
};
