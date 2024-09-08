#pragma once
#include "EditorAssetManager.h"
#include "Fussion/Core/Types.h"

#include <filesystem>

namespace fs = std::filesystem;

class Project {
public:
    static void initialize();
    static void save();
    static bool load(fs::path const& path);

    static auto name() -> std::string_view { return s_active_project->m_name; }
    static auto asset_manager() -> EditorAssetManager* { return s_active_project->m_asset_manager.get(); }
    static auto root() -> fs::path const& { return s_active_project->m_project_path; }
    static auto asset_registry() -> fs::path const& { return s_active_project->m_asset_registry_path; }
    static auto assets_folder() -> fs::path const& { return s_active_project->m_assets_folder_path; }
    static auto cache_folder() -> fs::path const& { return s_active_project->m_cache_folder_path; }
    static auto scripts_folder() -> fs::path const& { return s_active_project->m_scripts_folder_path; }
    static auto logs_folder() -> fs::path const& { return s_active_project->m_logs_folder_path; }

    /// Creates a new project at the specified location by creating all the necessary
    /// files and folders.
    /// @param path A path to an empty folder. If the path doesn't exist it will be created.
    /// @param name The name of the project. It <b>MUST</b> be path "friendly" because it will
    /// be used as the name of the project file.
    /// @returns A path the project file, which can used by Project::Load().
    static auto generate_project(fs::path const& path, std::string_view name) -> fs::path;

private:
    static Ptr<Project> s_active_project;

    Ptr<EditorAssetManager> m_asset_manager{};

    std::string m_name{ "Unnamed Project" };

    fs::path m_project_path;

    fs::path m_logs_folder_path;
    fs::path m_asset_registry_path;
    fs::path m_assets_folder_path;
    fs::path m_scripts_folder_path;
    fs::path m_cache_folder_path;
};
