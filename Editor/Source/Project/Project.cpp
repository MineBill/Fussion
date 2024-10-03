#include "EditorPCH.h"
#include "Project.h"

#include "Fussion/OS/FileSystem.h"

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Serialization/Json.h"
#include "Serialization/AssetSerializer.h"

Ptr<Project> Project::s_ActiveProject;
using namespace Fussion;

constexpr auto ASSETS_FOLDER = "AssetsFolder";
constexpr auto CACHE_FOLDER = "CacheFolder";
constexpr auto SCRIPTS_FOLDER = "ScriptsFolder";
constexpr auto ASSET_REGISTRY = "AssetRegistry";
constexpr auto LOGS_FOLDER = "Logs";

void Project::Initialize()
{
    s_ActiveProject = MakePtr<Project>();
    s_ActiveProject->m_AssetManager = MakePtr<EditorAssetManager>();

    AssetManager::SetActive(s_ActiveProject->m_AssetManager.get());
}

void Project::Save()
{
    s_ActiveProject->m_AssetManager->SaveToFile();
}

bool Project::Load(fs::path const& path)
{
    auto const data = FileSystem::ReadEntireFile(path);

    auto j = json::parse(*data, nullptr, true, true);

    s_ActiveProject->m_ProjectPath = path;
    auto const base = path.parent_path();

    if (j.contains("Name")) {
        s_ActiveProject->m_Name = j["Name"].get<std::string>();
    }

    if (j.contains(ASSETS_FOLDER)) {
        s_ActiveProject->m_AssetsFolderPath = base / j[ASSETS_FOLDER].get<std::string>();
    }
    if (j.contains(CACHE_FOLDER)) {
        s_ActiveProject->m_CacheFolderPath = base / j[CACHE_FOLDER].get<std::string>();
    }
    if (j.contains(SCRIPTS_FOLDER)) {
        s_ActiveProject->m_ScriptsFolderPath = base / j[SCRIPTS_FOLDER].get<std::string>();
    }
    if (j.contains(ASSET_REGISTRY)) {
        s_ActiveProject->m_AssetRegistryPath = base / j[ASSET_REGISTRY].get<std::string>();
    }
    if (j.contains(LOGS_FOLDER)) {
        s_ActiveProject->m_LogsFolderPath = base / j[LOGS_FOLDER].get<std::string>();
    }

    if (!exists(s_ActiveProject->m_AssetsFolderPath)) {
        LOG_ERRORF("AssetsFolder '{}' does not exist", s_ActiveProject->m_AssetsFolderPath.string());
    }
    if (!exists(s_ActiveProject->m_CacheFolderPath)) {
        LOG_ERRORF("CacheFolder '{}' does not exist", s_ActiveProject->m_CacheFolderPath.string());
    }
    if (!exists(s_ActiveProject->m_ScriptsFolderPath)) {
        LOG_ERRORF("ScriptsFolder '{}' does not exist", s_ActiveProject->m_ScriptsFolderPath.string());
    }
    if (!exists(s_ActiveProject->m_AssetRegistryPath)) {
        LOG_ERRORF("AssetRegistry '{}' does not exist", s_ActiveProject->m_AssetRegistryPath.string());
    }
    if (!exists(s_ActiveProject->m_LogsFolderPath)) {
        LOG_ERRORF("Logs folder '{}' does not exist", s_ActiveProject->m_LogsFolderPath.string());
    }
    s_ActiveProject->m_AssetManager->LoadFromFile();
    return true;
}

auto Project::GenerateProject(fs::path const& path, std::string_view name) -> fs::path
{

    auto fullPath = path / name;
    try {
        ordered_json project;
        fs::create_directories(fullPath);

        project["Name"] = name;
        project[ASSETS_FOLDER] = "Assets";
        project[CACHE_FOLDER] = "Cache";
        project[SCRIPTS_FOLDER] = "Scripts";
        project[ASSET_REGISTRY] = "AssetRegistry.json";
        project[LOGS_FOLDER] = "Logs";

        create_directory(fullPath / project[ASSETS_FOLDER]);
        create_directory(fullPath / project[CACHE_FOLDER]);
        create_directory(fullPath / project[SCRIPTS_FOLDER]);
        create_directory(fullPath / project[LOGS_FOLDER]);

        json assetRegistry;
        assetRegistry["$Type"] = "AssetRegistry";
        FileSystem::WriteEntireFile(fullPath / project[ASSET_REGISTRY], assetRegistry.dump(2));

        std::string nameWithExt(name);
        nameWithExt += ".fsnproj";

        auto projectPath = fullPath / nameWithExt;
        LOG_DEBUGF("Writing project file to {}", projectPath);
        FileSystem::WriteEntireFile(projectPath, project.dump(2));
        return projectPath;
    } catch (fs::filesystem_error const& error) {
        LOG_ERRORF("Failed to create directory for project creation: {}", error.what());
        return {};
    }
}
