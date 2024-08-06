#include "EditorAssetManager.h"
#include "EditorApplication.h"
#include "Project.h"
#include "Serialization/AssetSerializer.h"
#include "Serialization/SceneSerializer.h"
#include "Serialization/TextureSerializer.h"
#include "Serialization/MeshSerializer.h"
#include "Serialization/PbrMaterialSerializer.h"

#include "Fussion/OS/FileSystem.h"
#include "Fussion/Serialization/Json.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <fstream>

using namespace Fussion;

EditorAssetManager::EditorAssetManager()
{
    m_AssetSerializers[AssetType::Scene] = MakePtr<SceneSerializer>();
    m_AssetSerializers[AssetType::Texture2D] = MakePtr<TextureSerializer>();
    m_AssetSerializers[AssetType::Mesh] = MakePtr<MeshSerializer>();
    m_AssetSerializers[AssetType::PbrMaterial] = MakePtr<PbrMaterialSerializer>();
}

Asset* EditorAssetManager::GetAsset(AssetHandle handle, AssetType type)
{
    VERIFY(m_Registry.contains(handle), "The registry does not contain this asset handle: {}. Could it be that you are referencing a virtual asset?", handle);
    ZoneScoped;
    if (!IsAssetLoaded(handle)) {
        // Load asset first

        auto metadata = m_Registry[handle];

        // ??
        m_LoadedAssets[handle] = {};

        // TODO: What about assets that failed to load?
        m_LoadedAssets[handle] = m_AssetSerializers[type]->Load(metadata);
        m_LoadedAssets[handle]->SetHandle(handle);
        LOG_DEBUGF("Loading requested asset {} from '{}' of type {}", CAST(u64, handle), metadata.Path.string(), magic_enum::enum_name(metadata.Type));
    }
    return m_LoadedAssets[handle].get();
}

auto EditorAssetManager::GetAsset(std::string const& path, AssetType type) -> Asset*
{
    for (auto const& [handle, asset] : m_Registry) {
        if (asset.Path == path && asset.Type == type) {
            return GetAsset(handle, type);
        }
    }
    return nullptr;
}

bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
{
    return m_LoadedAssets.contains(handle);
}

bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
{
    return m_Registry.contains(handle);
}

bool EditorAssetManager::IsAssetVirtual(AssetHandle handle)
{
    return m_Registry[handle].IsVirtual;
}

AssetHandle EditorAssetManager::CreateVirtualAsset(Ref<Asset> const& asset, std::string_view name)
{
    auto handle = AssetHandle();
    m_Registry[handle] = AssetMetadata{
        .Type = asset->GetType(),
        .Path = "",
        .Name = std::string(name),
        .IsVirtual = true,
        .DontSerialize = true,
        .Handle = handle,
    };
    m_LoadedAssets[handle] = asset;

    return handle;
}

bool EditorAssetManager::IsPathAnAsset(std::filesystem::path const& path, bool include_virtual) const
{
    ZoneScoped;
    for (auto const& [id, metadata] : m_Registry) {
        if (!include_virtual && metadata.IsVirtual)
            continue;
        if (metadata.Path == path) {
            return true;
        }
    }
    return false;
}

AssetMetadata EditorAssetManager::GetMetadata(std::filesystem::path const& path) const
{
    ZoneScoped;
    for (auto const& [id, metadata] : m_Registry) {
        if (metadata.Path == path) {
            return metadata;
        }
    }
    return {};
}

AssetMetadata EditorAssetManager::GetMetadata(AssetHandle handle) const
{
    if (IsAssetHandleValid(handle)) {
        return m_Registry.at(handle);
    }
    return {};
}

void EditorAssetManager::RegisterAsset(std::filesystem::path const& path, Fussion::AssetType type)
{
    if (type == AssetType::Invalid) {
        LOG_WARNF("Ignoring Invalid asset type.");
        return;
    }
    auto pos = std::ranges::find_if(m_Registry, [&path](auto entry) -> bool { return entry.second.Path == path; });
    if (pos != m_Registry.end()) {
        LOG_ERRORF("Cannot register asset at path '{}', another asset lives there", path.string());
        return;
    }

    LOG_INFOF("Registering '{}' of type '{}'", path.string(), magic_enum::enum_name(type));

    Fsn::Uuid id;
    m_Registry[id] = AssetMetadata{
        .Type = type,
        .Path = path,
        .IsVirtual = false,
        .DontSerialize = false,
        .Handle = id,
    };

    Serialize();
}

void EditorAssetManager::SaveAsset(AssetHandle handle)
{
    m_AssetSerializers[m_Registry[handle].Type]->Save(m_Registry[handle], m_LoadedAssets[handle]);

    m_LoadedAssets[handle] = m_AssetSerializers[m_Registry[handle].Type]->Load(m_Registry[handle]);
    m_LoadedAssets[handle]->SetHandle(handle);
}

void EditorAssetManager::SaveAsset(Ref<Asset> const& asset)
{
    auto const handle = asset->GetHandle();
    m_AssetSerializers[m_Registry[handle].Type]->Save(m_Registry[handle], asset);

    m_LoadedAssets[handle] = m_AssetSerializers[m_Registry[handle].Type]->Load(m_Registry[handle]);
    m_LoadedAssets[handle]->SetHandle(handle);
}

void EditorAssetManager::Serialize()
{
    ZoneScoped;
    auto project = Project::ActiveProject();

    json j = {
        { "$Type", "AssetRegistry" },
    };

    u32 i = 0;
    for (auto const& [handle, metadata] : m_Registry) {
        if (metadata.IsVirtual || metadata.DontSerialize) {
            continue;
        }

        j["Assets"][i++] = {
            { "Handle", handle },
            { "Type", magic_enum::enum_name(metadata.Type) },
            { "Path", metadata.Path.string() },
            { "Name", metadata.Name },
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
        auto j = json::parse(*data);
        auto file_type = j["$Type"].get<std::string>();
        if (file_type != "AssetRegistry") {
            LOG_WARNF("The provided file file is not an AssetRegistry but: {}", file_type);
            return;
        }

        for (auto const& asset : j["Assets"]) {
            auto const handle = asset["Handle"].get<Fsn::Uuid>();
            auto const type = asset["Type"].get<std::string>();
            auto const asset_path = asset["Path"].get<std::string>();
            auto name = asset.value("Name", std::filesystem::path(asset_path).filename().string());

            Fsn::Uuid h{ handle };
            m_Registry[h] = AssetMetadata{
                .Type = *magic_enum::enum_cast<AssetType>(type),
                .Path = asset_path,
                .Name = name,
                .Handle = h,
            };
        }
    } catch (std::exception const& e) {
        LOG_ERRORF("Exception caught while deserialize asset registry: {}", e.what());
    }
}
