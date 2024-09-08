#include "EditorPCH.h"
#include "Project.h"

#include "Fussion/OS/FileSystem.h"

#include "Fussion/Assets/AssetManager.h"
#include "Serialization/AssetSerializer.h"
#include "Fussion/Serialization/Json.h"

Ptr<Project> Project::s_active_project;
using namespace Fussion;

constexpr auto ASSETS_FOLDER = "AssetsFolder";
constexpr auto CACHE_FOLDER = "CacheFolder";
constexpr auto SCRIPTS_FOLDER = "ScriptsFolder";
constexpr auto ASSET_REGISTRY = "AssetRegistry";
constexpr auto LOGS_FOLDER = "Logs";

void Project::initialize()
{
    s_active_project = make_ptr<Project>();
    s_active_project->m_asset_manager = make_ptr<EditorAssetManager>();

    AssetManager::set_active(s_active_project->m_asset_manager.get());
}

void Project::save()
{
    s_active_project->m_asset_manager->save_to_file();
}

bool Project::load(fs::path const& path)
{
    auto const data = FileSystem::read_entire_file(path);

    auto j = json::parse(*data, nullptr, true, true);

    s_active_project->m_project_path = path;
    auto const base = path.parent_path();

    if (j.contains("Name")) {
        s_active_project->m_name = j["Name"].get<std::string>();
    }

    if (j.contains(ASSETS_FOLDER)) {
        s_active_project->m_assets_folder_path = base / j[ASSETS_FOLDER].get<std::string>();
    }
    if (j.contains(CACHE_FOLDER)) {
        s_active_project->m_cache_folder_path = base / j[CACHE_FOLDER].get<std::string>();
    }
    if (j.contains(SCRIPTS_FOLDER)) {
        s_active_project->m_scripts_folder_path = base / j[SCRIPTS_FOLDER].get<std::string>();
    }
    if (j.contains(ASSET_REGISTRY)) {
        s_active_project->m_asset_registry_path = base / j[ASSET_REGISTRY].get<std::string>();
    }
    if (j.contains(LOGS_FOLDER)) {
        s_active_project->m_logs_folder_path = base / j[LOGS_FOLDER].get<std::string>();
    }

    if (!exists(s_active_project->m_assets_folder_path)) {
        LOG_ERRORF("AssetsFolder '{}' does not exist", s_active_project->m_assets_folder_path.string());
    }
    if (!exists(s_active_project->m_cache_folder_path)) {
        LOG_ERRORF("CacheFolder '{}' does not exist", s_active_project->m_cache_folder_path.string());
    }
    if (!exists(s_active_project->m_scripts_folder_path)) {
        LOG_ERRORF("ScriptsFolder '{}' does not exist", s_active_project->m_scripts_folder_path.string());
    }
    if (!exists(s_active_project->m_asset_registry_path)) {
        LOG_ERRORF("AssetRegistry '{}' does not exist", s_active_project->m_asset_registry_path.string());
    }
    if (!exists(s_active_project->m_logs_folder_path)) {
        LOG_ERRORF("Logs folder '{}' does not exist", s_active_project->m_logs_folder_path.string());
    }
    s_active_project->m_asset_manager->load_from_file();
    return true;
}

auto Project::generate_project(fs::path const& path, std::string_view name) -> fs::path
{
    ordered_json project;

    auto full_path = path / name;
    try {
        fs::create_directories(full_path);

        project["Name"] = name;
        project[ASSETS_FOLDER] = "Assets";
        project[CACHE_FOLDER] = "Cache";
        project[SCRIPTS_FOLDER] = "Scripts";
        project[ASSET_REGISTRY] = "AssetRegistry.json";
        project[LOGS_FOLDER] = "Logs";

        create_directory(full_path / project[ASSETS_FOLDER]);
        create_directory(full_path / project[CACHE_FOLDER]);
        create_directory(full_path / project[SCRIPTS_FOLDER]);
        create_directory(full_path / project[LOGS_FOLDER]);

        json asset_registry;
        asset_registry["$Type"] = "AssetRegistry";
        FileSystem::write_entire_file(full_path / project[ASSET_REGISTRY], asset_registry.dump(2));

        std::string name_with_ext(name);
        name_with_ext += ".fsnproj";

        auto project_path = full_path / name_with_ext;
        LOG_DEBUGF("Writing project file to {}", project_path);
        FileSystem::write_entire_file(project_path, project.dump(2));
        return project_path;
    } catch (fs::filesystem_error const& error) {
        LOG_ERRORF("Failed to create directory for project creation: {}", error.what());
        return {};
    }
}
