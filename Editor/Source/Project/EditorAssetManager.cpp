#include "EditorAssetManager.h"
#include "EditorApplication.h"
#include "Serialization/Helpers.h"
#include "Serialization/AssetSerializer.h"
#include "Project.h"

#include "Serialization/SceneSerializer.h"

#include <kdlpp.h>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <fstream>

#include "Fussion/OS/FileSystem.h"

using namespace Fussion;

EditorAssetManager::EditorAssetManager()
{
    m_AssetSerializers[AssetType::Scene] = MakePtr<SceneSerializer>();
}

Asset* EditorAssetManager::GetAsset(AssetHandle handle, AssetType type)
{
    if (!IsAssetLoaded(handle)) {
        // Load asset first

        auto metadata = m_Registry[handle];

        // ??
        m_LoadedAssets[handle] = {};

        auto ptr = m_AssetSerializers[type]->Load(metadata);
        EASSERT(ptr != nullptr);
        m_LoadedAssets[handle].reset(ptr);
        LOG_DEBUGF("Loading requested asset {} from '{}' of type {}", cast(u64, handle), metadata.Path.string(), magic_enum::enum_name(metadata.Type));
    }
    return m_LoadedAssets[handle].get();
}

bool EditorAssetManager::IsAssetLoaded(AssetHandle handle)
{
    return m_LoadedAssets.contains(handle);
}

bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle)
{
    return m_Registry.contains(handle);
}

bool EditorAssetManager::IsPathAnAsset(std::filesystem::path const& path) const
{
    for (auto const& [id, metadata] : m_Registry) {
        if (metadata.Path == path) {
            return true;
        }
    }
    return false;
}

AssetMetadata EditorAssetManager::GetMetadata(std::filesystem::path const& path) const
{
    for (auto const& [id, metadata] : m_Registry) {
        if (metadata.Path == path) {
            return metadata;
        }
    }
    return {};
}

void EditorAssetManager::SaveAsset(AssetHandle handle)
{
    m_AssetSerializers[m_Registry[handle].Type]->Save(m_Registry[handle], m_LoadedAssets[handle].get());
}

void EditorAssetManager::Serialize()
{
    ZoneScoped;
    auto project = Project::ActiveProject();

    using kdl::Document;
    using kdl::Node;
    using kdl::Value;

    std::vector<Node> nodes;
    auto& root = nodes.emplace_back("AssetRegistry");

    for (auto const& [handle, metadata] : m_Registry) {
        if (metadata.IsVirtual || metadata.DontSerialize) {
            continue;
        }

        auto& node = root.children().emplace_back("Asset");
        node.properties().insert_or_assign("Handle", std::to_string(cast(u64, handle)));
        node.properties().insert_or_assign("Type", magic_enum::enum_name(metadata.Type)).first->second.set_type_annotation("AssetType");
        node.properties().insert_or_assign("Path", metadata.Path.string());
    }

    auto const doc = Document({root});

    std::ofstream file(project->GetAssetRegistry());
    defer (file.close());

    file << doc.to_string();
}

void EditorAssetManager::Deserialize()
{
    ZoneScoped;
    auto const& path = Project::ActiveProject()->GetAssetRegistry();
    if (!std::filesystem::exists(path)) {
        LOG_WARNF("{} did not exist", path.string());
        return;
    }
    auto const data = FileSystem::ReadEntireFile(path);

    auto doc = kdl::parse(data);

    try {
        if (auto registry = FindNode(doc.nodes(), "AssetRegistry")) {
            for (auto const& node : registry->children()) {
                if (node.name() == "Asset") {
                    auto const handle = std::stoull(*GetProperty<std::string>(node, "Handle"));
                    auto const path = GetProperty<std::string>(node, "Path");
                    auto const type = GetProperty<std::string>(node, "Type");

                    m_Registry[Fsn::UUID(handle)] = AssetMetadata{
                        .Type = *magic_enum::enum_cast<AssetType>(*type),
                        .Path = *path,
                    };

                    LOG_DEBUGF("Adding asset '{}' with handle '{}' to registry.", *path, handle);
                }
            }
        }
    } catch (std::exception const& e) {
        LOG_ERRORF("Exception caught while deserialize asset registry: {}", e.what());
    }
}

