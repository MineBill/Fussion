#include "EditorAssetManager.h"
#include "EditorApplication.h"
#include "Serialization/AssetSerializer.h"
#include "Project.h"

#include "Serialization/SceneSerializer.h"
#include "Fussion/OS/FileSystem.h"
#include "Fussion/Serialization/Json.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <fstream>

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
        VERIFY(ptr != nullptr);
        m_LoadedAssets[handle].reset(ptr);
        LOG_DEBUGF("Loading requested asset {} from '{}' of type {}", CAST(u64, handle), metadata.Path.string(), magic_enum::enum_name(metadata.Type));
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

    // std::vector<Node> nodes;
    // auto& root = nodes.emplace_back("AssetRegistry");
    //
    // for (auto const& [handle, metadata] : m_Registry) {
    //     if (metadata.IsVirtual || metadata.DontSerialize) {
    //         continue;
    //     }
    //
    //     auto& node = root.children().emplace_back("Asset");
    //     node.properties().insert_or_assign("Handle", std::to_string(CAST(u64, handle)));
    //     node.properties().insert_or_assign("Type", magic_enum::enum_name(metadata.Type)).first->second.set_type_annotation("AssetType");
    //     node.properties().insert_or_assign("Path", metadata.Path.string());
    // }
    //
    // auto const doc = Document({root});
    json j = {
        {"$Type", "AssetRegistry"},
    };

    u32 i = 0;
    for (auto const& [handle, metadata] : m_Registry) {
        if (metadata.IsVirtual || metadata.DontSerialize) {
            continue;
        }

        j["Assets"][i++] = {
            {"Handle", handle},
            {"Type", magic_enum::enum_name(metadata.Type)},
            {"Path", metadata.Path.string()},
        };
    }

    FileSystem::WriteEntireFile(project->GetAssetRegistry(), j.dump(2));
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


    try {
        auto j = json::parse(data);
        auto type = j["$Type"].get<std::string>();
        if (type != "AssetRegistry") {
            LOG_WARNF("The provided file file is not an AssetRegistry but: {}", type);
            return;
        }

        for (auto const& asset : j["Assets"]) {
            auto const handle = asset["Handle"].get<Fsn::UUID>();
            auto const type = asset["Type"].get<std::string>();
            auto const path = asset["Path"].get<std::string>();

            Fsn::UUID h{ handle };
            m_Registry[h] = AssetMetadata{
                .Type = *magic_enum::enum_cast<AssetType>(type),
                .Path = path,
                .Handle = h,
            };
        }
    } catch (std::exception const& e) {
        LOG_ERRORF("Exception caught while deserialize asset registry: {}", e.what());
    }
}

