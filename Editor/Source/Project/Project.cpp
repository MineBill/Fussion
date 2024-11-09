#include "EditorPCH.h"
#include "Project.h"

#include "Fussion/OS/FileSystem.h"

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Serialization/Json.h"
#include "Fussion/Serialization/YamlSerializer.h"
#include "Serialization/AssetImporter.h"

Ptr<Project> Project::s_active_project;
using namespace Fussion;

constexpr auto ASSETS_FOLDER_KEY = "AssetsFolder";
constexpr auto CACHE_FOLDER_KEY = "CacheFolder";
constexpr auto SCRIPTS_FOLDER_KEY = "ScriptsFolder";
constexpr auto ASSET_REGISTRY_KEY = "AssetRegistry";
constexpr auto LOGS_FOLDER_KEY = "Logs";

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
    s_active_project->m_project_path = path;
    auto const base = path.parent_path();

    auto const data = FileSystem::read_entire_file(path);

    YamlDeserializer ds(*data);

    ds.read("Name", s_active_project->m_name);

    ds.read(ASSETS_FOLDER_KEY, s_active_project->m_assets_folder_path, base);
    ds.read(CACHE_FOLDER_KEY, s_active_project->m_cache_folder_path, base);
    ds.read(SCRIPTS_FOLDER_KEY, s_active_project->m_scripts_folder_path, base);
    ds.read(ASSET_REGISTRY_KEY, s_active_project->m_asset_registry_path, base);
    ds.read(LOGS_FOLDER_KEY, s_active_project->m_logs_folder_path, base);

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

    auto fullPath = path / name;
    try {
        YamlSerializer s;
        s.Initialize();

        fs::create_directories(fullPath);

        s.write("Name", name);
        s.write(ASSETS_FOLDER_KEY, "Assets");
        s.write(CACHE_FOLDER_KEY, "Cache");
        s.write(SCRIPTS_FOLDER_KEY, "Scripts");
        s.write(ASSET_REGISTRY_KEY, "AssetRegistry.fsn");
        s.write(LOGS_FOLDER_KEY, "Logs");

        create_directory(fullPath / "Assets");
        create_directory(fullPath / "Cache");
        create_directory(fullPath / "Scripts");
        create_directory(fullPath / "Logs");

        FileSystem::write_entire_file(fullPath / "AssetRegistry.fsn", "");

        std::string nameWithExt(name);
        nameWithExt += ".fsnproj";

        auto projectPath = fullPath / nameWithExt;
        LOG_DEBUGF("Writing project file to {}", projectPath);
        FileSystem::write_entire_file(projectPath, s.to_string());
        return projectPath;
    } catch (fs::filesystem_error const& error) {
        LOG_ERRORF("Failed to create directory for project creation: {}", error.what());
        return {};
    }
}
