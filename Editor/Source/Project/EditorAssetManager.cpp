#include "EditorAssetManager.h"
#include "EditorApplication.h"
#include "Project.h"
#include "Serialization/AssetSerializer.h"
#include "Serialization/SceneSerializer.h"
#include "Serialization/TextureSerializer.h"
#include "Serialization/MeshSerializer.h"
#include "Serialization/PbrMaterialSerializer.h"

#include "Fussion/OS/FileSystem.h"
#include "Fussion/RHI/ShaderCompiler.h"
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

    m_EditorWatcher = FileWatcher::Create(std::filesystem::current_path() / "Assets" / "Shaders");
    m_EditorWatcher->RegisterListener([this](std::filesystem::path const& path, FileWatcher::EventType type) {
        LOG_DEBUGF("Editor file changed: {} type: {}", path.string(), magic_enum::enum_name(type));
        if (type == FileWatcher::EventType::FileModified) {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(100ms);
            using namespace std::string_literals;
            auto full_path = std::filesystem::path("Assets") / "Shaders"s / path;
            auto meta = GetMetadata(full_path);
            if (meta.IsEmpty()) {
                return;
            }
            switch (meta->Type) {
            case AssetType::Shader: {
                auto shader = GetAsset(meta->Handle, AssetType::Shader)->As<ShaderAsset>();
                auto data = FileSystem::ReadEntireFile(meta->Path);
                auto result = RHI::ShaderCompiler::Compile(*data);
                if (result.HasValue()) {
                    *shader = ShaderAsset(shader->AssociatedRenderPass(), result->ShaderStages, result->Metadata);
                }
            }
            break;
            default:
                break;
            }

        }
    });
    m_EditorWatcher->Start();
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

AssetHandle EditorAssetManager::CreateVirtualAsset(Ref<Asset> const& asset, std::string_view name, std::filesystem::path const& path)
{
    auto handle = AssetHandle();
    m_Registry[handle] = EditorAssetMetadata{
        .Type = asset->GetType(),
        .Path = path,
        .Name = std::string(name),
        .IsVirtual = true,
        .DontSerialize = true,
        .Handle = handle,
    };
    m_LoadedAssets[handle] = asset;

    return handle;
}

AssetSettings* EditorAssetManager::GetAssetSettings(AssetHandle handle)
{
    if (m_AssetSettings[handle] == nullptr) {}
    return m_AssetSettings[handle].get();
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

Maybe<EditorAssetMetadata> EditorAssetManager::GetMetadata(std::filesystem::path const& path) const
{
    ZoneScoped;
    for (auto const& [id, metadata] : m_Registry) {
        if (metadata.Path == path) {
            return metadata;
        }
    }
    return {};
}

EditorAssetMetadata EditorAssetManager::GetMetadata(AssetHandle handle) const
{
    if (IsAssetHandleValid(handle)) {
        return m_Registry.at(handle);
    }
    return {};
}

void EditorAssetManager::RegisterAsset(std::filesystem::path const& path, AssetType type)
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

    Uuid id;
    m_Registry[id] = EditorAssetMetadata{
        .Type = type,
        .Path = path,
        .IsVirtual = false,
        .DontSerialize = false,
        .Handle = id,
        // .CustomMetadata =
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

void SerializeCustomMetadata(json& j, Ref<AssetMetadata> const& metadata)
{
    auto ptr = metadata->meta_poly_ptr();
    auto type = ptr.get_type().as_pointer().get_data_type().as_class();

    for (auto const& member : type.get_members()) {
        auto& m = j[member.get_name()];
        auto value = member.get(ptr);
        auto data_type = value.get_type().as_pointer().get_data_type();

        if (value.is<s8*>()) {
            m = *value.as<s8*>();
        } else if (value.is<s16*>()) {
            m = *value.as<s16*>();
        } else if (value.is<s32*>()) {
            m = *value.as<s32*>();
        } else if (value.is<s64*>()) {
            m = *value.as<s64*>();
        } else if (value.is<u8*>()) {
            m = *value.as<u8*>();
        } else if (value.is<u16*>()) {
            m = *value.as<u16*>();
        } else if (value.is<u32*>()) {
            m = *value.as<u32*>();
        } else if (value.is<f32*>()) {
            m = *value.as<f32*>();
        } else if (value.is<f64*>()) {
            m = *value.as<f64*>();
        } else if (value.is<bool*>()) {
            m = *value.as<bool*>();
        } else if (value.is<std::string*>()) {
            m = *value.as<std::string*>();
        } else if (value.is<Vector2*>()) {
            m = ToJson(*value.as<Vector2*>());
        } else if (value.is<Vector3*>()) {
            m = ToJson(*value.as<Vector3*>());
        } else if (value.is<Vector4*>()) {
            m = ToJson(*value.as<Vector4*>());
        } else if (value.is<Color*>()) {
            m = ToJson(*value.as<Color*>());
        } else if (data_type.is_enum()) {
            auto enum_type = data_type.as_enum();
            auto enum_value = enum_type.value_to_evalue(value);
            m = enum_value.get_name();
        }
    }
}

auto DeserializeCustomMetadata(json const& j, AssetType type) -> Ref<AssetMetadata>
{
    auto Deserialize = [j](meta_hpp::uvalue const& ptr) {
        if (!j.contains("CustomMetadata"))
            return;
        auto type = ptr.get_type().as_pointer().get_data_type().as_class();
        for (auto const& member : type.get_members()) {
            if (j["CustomMetadata"].contains(member.get_name())) {
                auto mem_value = member.get(ptr);
                auto data_type = mem_value.get_type().as_pointer().get_data_type();
                auto& value = j["CustomMetadata"][member.get_name()];

                if (mem_value.is<s8*>()) {
                    member.set(ptr, value.get<s8>());
                } else if (mem_value.is<s16*>()) {
                    member.set(ptr, value.get<s16>());
                } else if (mem_value.is<s32*>()) {
                    member.set(ptr, value.get<s32>());
                } else if (mem_value.is<s64*>()) {
                    member.set(ptr, value.get<s64>());
                } else if (mem_value.is<u8*>()) {
                    member.set(ptr, value.get<u8>());
                } else if (mem_value.is<u16*>()) {
                    member.set(ptr, value.get<u16>());
                } else if (mem_value.is<u32*>()) {
                    member.set(ptr, value.get<u32>());
                } else if (mem_value.is<f32*>()) {
                    member.set(ptr, value.get<f32>());
                } else if (mem_value.is<f64*>()) {
                    member.set(ptr, value.get<f64>());
                } else if (mem_value.is<bool*>()) {
                    member.set(ptr, value.get<bool>());
                } else if (mem_value.is<std::string*>()) {
                    member.set(ptr, value.get<std::string>());
                } else if (mem_value.is<Vector2*>()) {
                    member.set(ptr, value.get<Vector2>());
                } else if (mem_value.is<Vector3*>()) {
                    member.set(ptr, value.get<Vector3>());
                } else if (mem_value.is<Vector4*>()) {
                    member.set(ptr, value.get<Vector4>());
                } else if (mem_value.is<Color*>()) {
                    member.set(ptr, value.get<Color>());
                } else if (data_type.is_enum()) {
                    auto enum_type = data_type.as_enum();
                    auto enum_value = enum_type.name_to_evalue(value.get<std::string>());
                    member.set(ptr, enum_value);
                }
            }
        }
    };
    using enum AssetType;
    switch (type) {
    case Texture2D: {
        auto meta = MakeRef<Texture2DMetadata>();
        Deserialize(meta->meta_poly_ptr());
        return meta;
    }
    default:
        break;
    }
    return nullptr;
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

        j["Assets"][i] = {
            { "Handle", handle },
            { "Type", magic_enum::enum_name(metadata.Type) },
            { "Path", metadata.Path.string() },
            { "Name", metadata.Name },
        };

        if (metadata.CustomMetadata != nullptr) {
            SerializeCustomMetadata(j["Assets"][i++]["CustomMetadata"], metadata.CustomMetadata);
        }
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

            Uuid h{ handle };
            m_Registry[h] = EditorAssetMetadata{
                .Type = *magic_enum::enum_cast<AssetType>(type),
                .Path = asset_path,
                .Name = name,
                .Handle = h,
            };

            m_Registry[h].CustomMetadata = DeserializeCustomMetadata(asset, m_Registry[h].Type);
        }
    } catch (std::exception const& e) {
        LOG_ERRORF("Exception caught while deserialize asset registry: {}", e.what());
    }
}
