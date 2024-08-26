#include "EditorPCH.h"
#include "Project.h"

#include "Fussion/OS/FileSystem.h"

#include "Fussion/Assets/AssetManager.h"
#include "Serialization/AssetSerializer.h"
#include "Fussion/Serialization/Json.h"

Ptr<Project> Project::s_ActiveProject;
using namespace Fussion;

constexpr auto AssetsFolder = "AssetsFolder";
constexpr auto CacheFolder = "CacheFolder";
constexpr auto ScriptsFolder = "ScriptsFolder";
constexpr auto AssetRegistry = "AssetRegistry";
constexpr auto LogsFolder = "Logs";

void Project::Initialize()
{
    s_ActiveProject = MakePtr<Project>();
    s_ActiveProject->m_AssetManager = MakeRef<EditorAssetManager>();

    AssetManager::SetActive(s_ActiveProject->m_AssetManager);
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

    if (j.contains(AssetsFolder)) {
        s_ActiveProject->m_AssetsFolderPath = base / j[AssetsFolder].get<std::string>();
    }
    if (j.contains(CacheFolder)) {
        s_ActiveProject->m_CacheFolderPath = base / j[CacheFolder].get<std::string>();
    }
    if (j.contains(ScriptsFolder)) {
        s_ActiveProject->m_ScriptsFolderPath = base / j[ScriptsFolder].get<std::string>();
    }
    if (j.contains(AssetRegistry)) {
        s_ActiveProject->m_AssetRegistryPath = base / j[AssetRegistry].get<std::string>();
    }
    if (j.contains(LogsFolder)) {
        s_ActiveProject->m_LogsFolderPath = base / j[LogsFolder].get<std::string>();
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
    ordered_json project;

    project["Name"] = name;
    project[AssetsFolder] = "Assets";
    project[CacheFolder] = "Cache";
    project[ScriptsFolder] = "Scripts";
    project[AssetRegistry] = "AssetRegistry.json";
    project[LogsFolder] = "Logs";

    create_directory(path / project[AssetsFolder]);
    create_directory(path / project[CacheFolder]);
    create_directory(path / project[ScriptsFolder]);
    create_directory(path / project[LogsFolder]);

    json asset_registry;
    asset_registry["$Type"] = "AssetRegistry";
    FileSystem::WriteEntireFile(path / project[AssetRegistry], asset_registry.dump(2));

    std::string name_with_ext(name);
    name_with_ext += ".fsnproj";

    auto project_path = path / name_with_ext;
    LOG_DEBUGF("Writing project file to {}", project_path);
    FileSystem::WriteEntireFile(project_path, project.dump(2));
    return project_path;
}
