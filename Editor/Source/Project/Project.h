#pragma once
#include <filesystem>

#include "EditorAssetManager.h"
#include "Fussion/Core/Result.h"
#include "Fussion/Core/Types.h"

class Project
{
public:
    static Ref<Project> ActiveProject() {
        return s_ActiveProject;
    }

    static void Save(std::filesystem::path path = {});
    static bool Load(std::filesystem::path path);

    Ref<EditorAssetManager>& GetAssetManager() { return m_AssetManager; }

    std::filesystem::path const& GetRoot() const {
        return m_ProjectPath;
    }

    std::filesystem::path const& GetAssetRegistry() const {
        return m_AssetRegistryPath;
    }

    std::filesystem::path const& GetAssetsFolder() const {
        return m_AssetsFolderPath;
    }

    std::filesystem::path const& GetCacheFolder() const {
        return m_CacheFolderPath;
    }

private:
    static Ref<Project> s_ActiveProject;

    Ref<EditorAssetManager> m_AssetManager{};

    std::string m_Name;

    std::filesystem::path m_ProjectPath;
    std::filesystem::path m_AssetRegistryPath;
    std::filesystem::path m_AssetsFolderPath;
    std::filesystem::path m_CacheFolderPath;
};
