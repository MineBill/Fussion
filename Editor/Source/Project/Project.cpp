#include "epch.h"
#include "Project.h"

#include "Fussion/OS/FileSystem.h"
#include <magic_enum/magic_enum.hpp>

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Core/Result.h"
#include "Serialization/AssetSerializer.h"
#include "Fussion/Serialization/Json.h"

Ref<Project> Project::s_ActiveProject;
using namespace Fussion;

void Project::Initialize()
{
    s_ActiveProject = MakeRef<Project>();
    s_ActiveProject->m_AssetManager = MakeRef<EditorAssetManager>();

    AssetManager::SetActive(s_ActiveProject->m_AssetManager);
}

void Project::Save(std::filesystem::path path)
{
    s_ActiveProject->m_AssetManager->Serialize();
}

bool Project::Load(std::filesystem::path const& path)
{
    auto const data = FileSystem::ReadEntireFile(path);

    auto j = json::parse(*data, nullptr, true, true);

    s_ActiveProject->m_ProjectPath = path;
    auto const base = path.parent_path();

    // TODO: Check that all the paths exist.
    if (j.contains("AssetsFolder")) {
        s_ActiveProject->m_AssetsFolderPath = base / j["AssetsFolder"].get<std::string>();
    }
    if (j.contains("CacheFolder")) {
        s_ActiveProject->m_CacheFolderPath = base / j["CacheFolder"].get<std::string>();
    }
    if (j.contains("ScriptsFolder")) {
        s_ActiveProject->m_ScriptsFolderPath = base / j["ScriptsFolder"].get<std::string>();
    }
    if (j.contains("AssetRegistry")) {
        s_ActiveProject->m_AssetRegistryPath = base / j["AssetRegistry"].get<std::string>();
    }
    if (j.contains("Logs")) {
        s_ActiveProject->m_LogsFolderPath = base / j["Logs"].get<std::string>();
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
    s_ActiveProject->m_AssetManager->Deserialize();
    return true;
}
