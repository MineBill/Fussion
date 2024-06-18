#include "Project.h"

#include "Fussion/OS/FileSystem.h"
#include <magic_enum/magic_enum.hpp>

#include "Fussion/Assets/AssetManager.h"
#include "Fussion/Core/Result.h"
#include "Serialization/AssetSerializer.h"
#include "Fussion/Serialization/Json.h"

Ref<Project> Project::s_ActiveProject;

void Project::Save(std::filesystem::path path)
{
    s_ActiveProject->m_AssetManager->Serialize();
    // m_AssetManager->Serialize();
}

bool Project::Load(std::filesystem::path path)
{
    using namespace Fussion;
    s_ActiveProject = MakeRef<Project>();

    auto const data = FileSystem::ReadEntireFile(path);

    auto j = json::parse(data, nullptr, true, true);

    s_ActiveProject->m_ProjectPath = path;
    auto const base = path.parent_path();

    if (j.contains("AssetsFolder")) {
        s_ActiveProject->m_AssetsFolderPath = base / j["AssetsFolder"].get<std::string>();
    }
    if (j.contains("CacheFolder")) {
        s_ActiveProject->m_CacheFolderPath = base / j["CacheFolder"].get<std::string>();
    }
    if (j.contains("AssetRegistry")) {
        s_ActiveProject->m_AssetRegistryPath = base / j["AssetRegistry"].get<std::string>();
    }

    // @todo Check that all the paths exist.

    s_ActiveProject->m_AssetManager = MakeRef<EditorAssetManager>();
    s_ActiveProject->m_AssetManager->Deserialize();

    AssetManager::SetActive(s_ActiveProject->m_AssetManager);
    return true;
}

