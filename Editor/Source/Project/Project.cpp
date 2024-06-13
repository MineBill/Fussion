#include "Project.h"

#include "kdlpp.h"
#include "Fussion/OS/FileSystem.h"
#include <magic_enum/magic_enum.hpp>

#include "Fussion/Core/Result.h"
#include "Serialization/Helpers.h"
#include "Serialization/AssetSerializer.h"

using kdl::Document;
using kdl::Node;

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

    auto doc = kdl::parse(data);

    s_ActiveProject->m_ProjectPath = path;
    auto const base = path.parent_path();

    if (auto proj = FindNode(doc.nodes(), "Project")) {
        if (auto a = FindNode(proj->children(), "AssetsFolder")) {
            if (auto value = GetArgAt<std::string>(*a, 0, kdl::Type::String)) {
                s_ActiveProject->m_AssetsFolderPath = base / *value;
            }
        }

        if (auto a = FindNode(proj->children(), "CacheFolder")) {
            if (auto value = GetArgAt<std::string>(*a, 0, kdl::Type::String)) {
                s_ActiveProject->m_CacheFolderPath = base / *value;
            }
        }

        if (auto a = FindNode(proj->children(), "AssetRegistry")) {
            if (auto value = GetArgAt<std::string>(*a, 0, kdl::Type::String)) {
                s_ActiveProject->m_AssetRegistryPath = base / *value;
            }
        }
    } else {
        LOG_ERRORF("Could not find Project node in project file");
        return false;
    }

    // @todo Check that all the paths exist.

    s_ActiveProject->m_AssetManager = MakeRef<EditorAssetManager>();
    s_ActiveProject->m_AssetManager->Deserialize();

    AssetManager::SetActive(s_ActiveProject->m_AssetManager);
    return true;
}

