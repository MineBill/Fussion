#include "EditorPCH.h"
#include "EditorAssetManager.h"
#include "EditorApplication.h"
#include "Project.h"
#include "Fussion/Assets/Model.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Serialization/AssetSerializer.h"
#include "Serialization/SceneSerializer.h"
#include "Serialization/TextureSerializer.h"
#include "Serialization/MeshSerializer.h"
#include "Serialization/PbrMaterialSerializer.h"

#include "Fussion/OS/FileSystem.h"
#include "Fussion/RHI/ShaderCompiler.h"
#include "Fussion/Serialization/Json.h"
#include "Fussion/Serialization/JsonSerializer.h"

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <future>

using namespace Fussion;

// void EditorAssetMetadata::Serialize(Serializer& ctx) const
// {
//     if (IsVirtual || DontSerialize)
//         return;
//     ISerializable::Serialize(ctx);
//     FSN_SERIALIZE_MEMBER(Type);
//     FSN_SERIALIZE_MEMBER(Path);
//     FSN_SERIALIZE_MEMBER(Name);
// }
//
// void EditorAssetMetadata::Deserialize(Deserializer& ctx)
// {
//     ISerializable::Deserialize(ctx);
//     FSN_DESERIALIZE_MEMBER(Type);
//     FSN_DESERIALIZE_MEMBER(Path);
//     FSN_DESERIALIZE_MEMBER(Name);
// }

WorkerPool::WorkerPool(EditorAssetManager* asset_manager): m_AssetManager(asset_manager)
{
    auto max_threads = std::thread::hardware_concurrency();
    LOG_INFOF("Creating {} worker threads for background asset loading.", max_threads);
    for (u32 i = 0; i < max_threads; i++) {
        m_Workers.emplace_back(&WorkerPool::Work, this, i);
    }
}

void WorkerPool::Work(s32 index)
{
    auto pool = RHI::Device::Instance()->CreateCommandPool();
    RHI::ThreadLocalCommandPool = pool;

    // NOTE: This probably doesn't hurt much since these are pointers to functions, no data are created.
    std::map<AssetType, Ptr<AssetSerializer>> asset_serializers{};
    asset_serializers[AssetType::Texture2D] = MakePtr<TextureSerializer>();
    asset_serializers[AssetType::Model] = MakePtr<MeshSerializer>();

    auto MakeAsset = [](AssetType type) -> Ref<Asset> {
        switch (type) {
        case AssetType::Model:
            return MakeRef<Model>();
        case AssetType::PbrMaterial:
            return MakeRef<PbrMaterial>();
        case AssetType::Scene:
            return MakeRef<Scene>();
        case AssetType::Texture2D:
            return MakeRef<Texture2D>();
        default:
            UNREACHABLE;
        }
    };

    LOG_INFOF("Worker({}) started", index);

    while (true) {
        Maybe<EditorAssetMetadata> task;
        {
            std::unique_lock lock(m_Mutex);
            // This condition variable will unblock iff m_Tasks is not empty.
            // This is to prevent a case where the condition is notified but no tasks are available.
            m_ConditionVariable.wait(lock, [this] { return !m_Tasks.empty(); });

            task = m_Tasks.front();
            m_Tasks.pop();
        }

        if (task.HasValue()) {
            LOG_INFOF("Worker({}) was notified about a new task: {}", index, task->Path.string());

            if (asset_serializers.contains(task->Type)) {
                auto asset = asset_serializers[task->Type]->Load(*task);
                asset->SetHandle(task->Handle);
                LoadedAssets.Access([&](auto& queue) {
                    queue.push(asset);
                });
            } else {
                auto asset = MakeAsset(task->Type);
                if (auto json_string = FileSystem::ReadEntireFile(Project::ActiveProject()->GetAssetsFolder() / task->Path)) {
                    JsonDeserializer ds(*json_string);
                    asset->Deserialize(ds);
                    asset->SetHandle(task->Handle);
                    LoadedAssets.Access([&](auto& queue) {
                        queue.push(asset);
                    });
                }
            }

            // // TODO: This whole thing screams thread safety :) m_LoadedAssets probably needs to be guarded too.
            // m_AssetManager->m_LoadedAssets[task->Handle] = m_AssetManager->m_AssetSerializers[task->Type]->Load(*task);
            //
            // m_AssetManager->m_Registry.Access([&](EditorAssetManager::Registry& registry) {
            //     registry[task->Handle].LoadState = AssetLoadState::Loaded;
            // });
        }
    }
}

void WorkerPool::Load(EditorAssetMetadata const& metadata)
{
    {
        std::lock_guard lock(m_Mutex);
        m_Tasks.push(metadata);
    }

    m_ConditionVariable.notify_one();
}

EditorAssetManager::EditorAssetManager()
    : m_EditorWatcher(FileWatcher::Create(std::filesystem::current_path() / "Assets" / "Shaders"))
{
    m_AssetImporters[AssetType::Texture2D] = MakePtr<TextureSerializer>();
    m_AssetImporters[AssetType::Model] = MakePtr<MeshSerializer>();

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

EditorAssetManager::~EditorAssetManager() = default;

Asset* EditorAssetManager::GetAsset(AssetHandle handle, AssetType type)
{
    // VERIFY(m_Registry.contains(handle), "The registry does not contain this asset handle: {}. Could it be that you are referencing a virtual asset?", handle);
    ZoneScoped;
    if (!IsAssetLoaded(handle)) {
        if (IsAssetLoading(handle)) {
            return nullptr;
        }
        LoadAsset(handle, type);
        return nullptr;
    }
    return m_LoadedAssets[handle].get();
}

auto EditorAssetManager::GetAsset(std::string const& path, AssetType type) -> Asset*
{
    return m_Registry.Access([&](Registry const& registry) -> Asset* {
        for (auto const& [handle, asset] : registry) {
            if (asset.Path == path && asset.Type == type) {
                return GetAsset(handle, type);
            }
        }
        return nullptr;
    });
}

bool EditorAssetManager::IsAssetLoaded(AssetHandle handle)
{
    CheckForLoadedAssets();
    return m_Registry.Access([&](Registry const& registry) {
        auto const& metadata = registry.at(handle);
        return metadata.LoadState == AssetLoadState::Loaded;
    });
}

bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
{
    return m_Registry.Access([&](Registry const& registry) {
        return registry.contains(handle);
    });
}

bool EditorAssetManager::IsAssetVirtual(AssetHandle handle)
{
    return m_Registry.Access([&](Registry const& registry) {
        if (!registry.contains(handle))
            return false;
        return registry.at(handle).IsVirtual;
    });
}

AssetHandle EditorAssetManager::CreateVirtualAsset(Ref<Asset> const& asset, std::string_view name, std::filesystem::path const& path)
{
    auto handle = AssetHandle();
    m_Registry.Access([&](Registry& registry) {
        registry[handle] = EditorAssetMetadata{
            .Type = asset->GetType(),
            .Path = path,
            .Name = std::string(name),
            .IsVirtual = true,
            .DontSerialize = true,
            .LoadState = AssetLoadState::Loaded,
            .Handle = handle,
        };
    });
    m_LoadedAssets[handle] = asset;

    return handle;
}

AssetMetadata* EditorAssetManager::GetAssetMetadata(AssetHandle handle)
{
    if (!IsAssetHandleValid(handle))
        return nullptr;

    return m_Registry.Access([&](Registry& registry) {
        return registry[handle].CustomMetadata.get();
    });
}

bool EditorAssetManager::IsAssetLoading(AssetHandle handle)
{
    return m_Registry.Access([&](Registry const& registry) {
        auto const& metadata = registry.at(handle);
        return metadata.LoadState == AssetLoadState::Loading;
    });
}

bool EditorAssetManager::IsPathAnAsset(std::filesystem::path const& path, bool include_virtual) const
{
    ZoneScoped;
    return m_Registry.Access([&](Registry const& registry) {
        for (auto const& [id, metadata] : registry) {
            (void)id;
            if (!include_virtual && metadata.IsVirtual)
                continue;
            if (metadata.Path == path) {
                return true;
            }
        }
        return false;
    });
}

Maybe<EditorAssetMetadata> EditorAssetManager::GetMetadata(std::filesystem::path const& path) const
{
    ZoneScoped;
    return m_Registry.Access([&](Registry const& registry) -> EditorAssetMetadata {
        for (auto const& [id, metadata] : registry) {
            (void)id;
            if (metadata.Path == path) {
                return metadata;
            }
        }
        return {};
    });
}

EditorAssetMetadata EditorAssetManager::GetMetadata(AssetHandle handle) const
{
    if (IsAssetHandleValid(handle)) {
        return m_Registry.Access([&](Registry const& registry) {
            return registry.at(handle);
        });
    }
    return {};
}

auto MetadataForAsset(AssetType type) -> Ref<AssetMetadata>
{
    using enum AssetType;
    switch (type) {
    case Texture2D: {
        return MakeRef<Texture2DMetadata>();
    }
    default:
        break;
    }
    return nullptr;
}

void EditorAssetManager::RegisterAsset(std::filesystem::path const& path, AssetType type)
{
    if (type == AssetType::Invalid) {
        LOG_WARNF("Ignoring Invalid asset type.");
        return;
    }
    m_Registry.Access([&](Registry& registry) {
        auto pos = std::ranges::find_if(registry, [&path](auto entry) -> bool { return entry.second.Path == path; });
        if (pos != registry.end()) {
            LOG_ERRORF("Cannot register asset at path '{}', another asset lives there", path.string());
            return;
        }

        LOG_INFOF("Registering '{}' of type '{}'", path.string(), magic_enum::enum_name(type));

        Uuid id;
        registry[id] = EditorAssetMetadata{
            .Type = type,
            .Path = path,
            .Name = path.filename().string(),
            .IsVirtual = false,
            .DontSerialize = false,
            .Handle = id,
            .CustomMetadata = MetadataForAsset(type),
        };
    });

    SaveToFile();
}

void EditorAssetManager::SaveAsset(AssetHandle handle)
{
    auto meta = m_Registry.Access([&](Registry& registry) {
        return registry[handle];
    });

    if (m_AssetImporters.contains(meta.Type)) {
        m_AssetImporters[meta.Type]->Save(meta, m_LoadedAssets[handle]);
        m_LoadedAssets[handle] = m_AssetImporters[meta.Type]->Load(meta);
        m_LoadedAssets[handle]->SetHandle(handle);
    } else {
        JsonSerializer js;
        js.Initialize();

        m_LoadedAssets[handle]->Serialize(js);

        auto path = Project::ActiveProject()->GetAssetsFolder() / meta.Path;
        FileSystem::WriteEntireFile(path, js.ToString());
    }
}

void EditorAssetManager::SaveAsset(Ref<Asset> const& asset)
{
    auto const handle = asset->GetHandle();
    m_Registry.Access([&](Registry& registry) {
        m_AssetImporters[registry[handle].Type]->Save(registry[handle], asset);

        m_LoadedAssets[handle] = m_AssetImporters[registry[handle].Type]->Load(registry[handle]);
        m_LoadedAssets[handle]->SetHandle(handle);
    });
}

auto DeserializeCustomMetadata(json const& j, AssetType type) -> Ref<AssetMetadata>
{
    using enum AssetType;
    switch (type) {
    case Texture2D: {
        auto meta = MakeRef<Texture2DMetadata>();
        JsonDeserializer ds = JsonDeserializer::FromJsonObject(j);

        ds.Read("CustomMetadata", *meta);
        return meta;
    }
    default:
        break;
    }
    return nullptr;
}

void EditorAssetManager::SaveToFile()
{
    ZoneScoped;
    auto project = Project::ActiveProject();

    json j = {
        { "$Type", "AssetRegistry" },
    };

    u32 i = 0;
    m_Registry.Access([&](Registry const& registry) {
        for (auto const& [handle, metadata] : registry) {
            if (metadata.IsVirtual || metadata.DontSerialize) {
                continue;
            }

            auto index = i++;
            j["Assets"][index] = {
                { "Handle", handle },
                { "Type", magic_enum::enum_name(metadata.Type) },
                { "Path", metadata.Path.string() },
                { "Name", metadata.Name },
            };

            if (metadata.CustomMetadata != nullptr) {
                auto ptr = metadata.CustomMetadata->meta_poly_ptr();
                auto class_type = ptr.get_type().as_pointer().get_data_type().as_class();
                auto name = class_type.get_metadata().at("Name").as<std::string>();
                j["Assets"][index]["CustomMetadata"] = SerializeNativeClass(class_type, std::move(ptr));
                j["Assets"][index]["CustomMetadata"]["$Type"] = name;
            }
        }

        FileSystem::WriteEntireFile(project->GetAssetRegistry(), j.dump(2));
    });
}

void EditorAssetManager::LoadFromFile()
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

            m_Registry.Access([&](Registry& registry) {
                registry[h] = EditorAssetMetadata{
                    .Type = *magic_enum::enum_cast<AssetType>(type),
                    .Path = asset_path,
                    .Name = name,
                    .Handle = h,
                };

                registry[h].CustomMetadata = DeserializeCustomMetadata(asset, registry[h].Type);
            });
        }
    } catch (std::exception const& e) {
        LOG_ERRORF("Exception caught while deserialize asset registry: {}", e.what());
    }
}

void EditorAssetManager::RefreshAsset(AssetHandle handle)
{
    if (!IsAssetHandleValid(handle) || !IsAssetLoaded(handle))
        return;

    SaveAsset(handle);
}

void EditorAssetManager::CheckForLoadedAssets()
{
    m_WorkerPool.LoadedAssets.Access([this](std::queue<Ref<Asset>>& queue) {
        while (!queue.empty()) {
            auto asset = queue.front();
            queue.pop();

            auto handle = asset->GetHandle();
            m_LoadedAssets[handle] = asset;

            m_Registry.Access([&](Registry& registry) {
                registry[handle].LoadState = AssetLoadState::Loaded;
            });
        }
    });
}

void EditorAssetManager::Serialize(Serializer& ctx) const
{
    ISerializable::Serialize(ctx);
    auto project = Project::ActiveProject();

    m_Registry.Access([&](Registry const& registry) {
        // for (auto const& [handle, metadata] : registry) {
        //     if (metadata.IsVirtual || metadata.DontSerialize) {
        //         continue;
        //     }
        //
        //     auto index = i++;
        //     j["Assets"][index] = {
        //         { "Handle", handle },
        //         { "Type", magic_enum::enum_name(metadata.Type) },
        //         { "Path", metadata.Path.string() },
        //         { "Name", metadata.Name },
        //     };
        //
        //     if (metadata.CustomMetadata != nullptr) {
        //         auto ptr = metadata.CustomMetadata->meta_poly_ptr();
        //         auto class_type = ptr.get_type().as_pointer().get_data_type().as_class();
        //         auto name = class_type.get_metadata().at("Name").as<std::string>();
        //         j["Assets"][index]["CustomMetadata"] = SerializeNativeClass(class_type, std::move(ptr));
        //         j["Assets"][index]["CustomMetadata"]["$Type"] = name;
        //     }
        // }
        ctx.BeginArray("Assets", registry.size());
        for (auto const& metadata : registry | std::views::values) {
            if (metadata.IsVirtual || metadata.DontSerialize)
                continue;
            ctx.BeginObject("", 0);
            ctx.Write("Name", metadata.Name);
            ctx.Write("Handle", metadata.Handle);
            ctx.Write("Type", metadata.Type);
            ctx.Write("Path", metadata.Path);

            if (metadata.CustomMetadata != nullptr) {
                ctx.Write("CustomMetadata", *metadata.CustomMetadata);
            }
            ctx.EndObject();
        }
        ctx.EndArray();

    });
}

void EditorAssetManager::Deserialize(Deserializer& ctx)
{
    ISerializable::Deserialize(ctx);
    size_t size;
    ctx.BeginArray("Assets", size);
    m_Registry.Access([&](Registry& registry) {
        registry.reserve(size);

        for (size_t i = 0; i < size; i++) {
            size_t _s;
            ctx.BeginObject("", _s);

            EditorAssetMetadata metadata{};
            ctx.Read("Name", metadata.Name);
            ctx.Read("Handle", metadata.Handle);
            ctx.Read("Type", metadata.Type);
            ctx.Read("Path", metadata.Path);

            // size_t custom_metadata_size;
            // if (ctx.BeginObject("CustomMetadata", custom_metadata_size) {
            //     ctx.Read("CustomMetadata", metadata);
            //     ctx.EndObject();
            // }

            switch (metadata.Type) {
            case AssetType::Texture2D: {
                auto meta = MakeRef<Texture2DMetadata>();
                ctx.Read("CustomMetadata", *meta);
                metadata.CustomMetadata = meta;
            }
            break;
            default:
                break;
            }

            ctx.EndObject();
        }
    });
    ctx.EndArray();
}

void EditorAssetManager::LoadAsset(AssetHandle handle, AssetType type)
{
    LOG_DEBUGF("LoadAsset[{}, {}]", handle, magic_enum::enum_name(type));
    m_Registry.Access([&](Registry& registry) {
        registry[handle].LoadState = AssetLoadState::Loading;
#if 0
        auto metadata = registry[handle];
        // ??
        m_LoadedAssets[handle] = {};

        // TODO: What about assets that failed to load?
        m_LoadedAssets[handle] = m_AssetSerializers[type]->Load(metadata);
        m_LoadedAssets[handle]->SetHandle(handle);

        registry[handle].LoadState = AssetLoadState::Loaded;

        LOG_DEBUGF("Loaded asset '{}' from '{}' of type '{}'", CAST(u64, handle), metadata.Path.string(), magic_enum::enum_name(metadata.Type));
#else
        m_WorkerPool.Load(registry[handle]);
#endif

    });

}
