#include "EditorPCH.h"
#include "Project.h"

#include "Fussion/OS/FileSystem.h"

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Serialization/Json.h"
#include "Fussion/Serialization/YamlSerializer.h"
#include "Serialization/AssetImporter.h"

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
    s_ActiveProject->m_ProjectPath = path;
    auto const base = path.parent_path();

    auto const data = FileSystem::ReadEntireFile(path);

    YamlDeserializer ds(*data);

    ds.Read("Name", s_ActiveProject->m_Name);

    ds.Read(ASSETS_FOLDER, s_ActiveProject->m_AssetsFolderPath, base);
    ds.Read(CACHE_FOLDER, s_ActiveProject->m_CacheFolderPath, base);
    ds.Read(SCRIPTS_FOLDER, s_ActiveProject->m_ScriptsFolderPath, base);
    ds.Read(ASSET_REGISTRY, s_ActiveProject->m_AssetRegistryPath, base);
    ds.Read(LOGS_FOLDER, s_ActiveProject->m_LogsFolderPath, base);

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
        YamlSerializer s;
        s.Initialize();

        fs::create_directories(fullPath);

        // project["Name"] = name;
        // project[ASSETS_FOLDER] = "Assets";
        // project[CACHE_FOLDER] = "Cache";
        // project[SCRIPTS_FOLDER] = "Scripts";
        // project[ASSET_REGISTRY] = "AssetRegistry.json";
        // project[LOGS_FOLDER] = "Logs";

        s.Write("Name", name);
        s.Write(ASSETS_FOLDER, "Assets");
        s.Write(CACHE_FOLDER, "Cache");
        s.Write(SCRIPTS_FOLDER, "Scripts");
        s.Write(ASSET_REGISTRY, "AssetRegistry.fsn");
        s.Write(LOGS_FOLDER, "Logs");

        create_directory(fullPath / "Assets");
        create_directory(fullPath / "Cache");
        create_directory(fullPath / "Scripts");
        create_directory(fullPath / "Logs");

        FileSystem::WriteEntireFile(fullPath / "AssetRegistry.fsn", "");

        std::string nameWithExt(name);
        nameWithExt += ".fsnproj";

        auto projectPath = fullPath / nameWithExt;
        LOG_DEBUGF("Writing project file to {}", projectPath);
        FileSystem::WriteEntireFile(projectPath, s.ToString());
        return projectPath;
    } catch (fs::filesystem_error const& error) {
        LOG_ERRORF("Failed to create directory for project creation: {}", error.what());
        return {};
    }
}
