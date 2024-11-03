#include "EditorPCH.h"

#include "EditorAssetManager.h"

#include "EditorApplication.h"
#include "Fussion/Assets/Model.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/Assets/ShaderAsset.h"
#include "Fussion/Serialization/BinarySerializer.h"
#include "Fussion/Serialization/YamlSerializer.h"
#include "Project.h"
#include "Serialization/AssetImporter.h"
#include "Serialization/MeshImporter.h"
#include "Serialization/TextureImporter.h"

#include <Fussion/OS/FileSystem.h>
#include <Fussion/Scene/Scene.h>
#include <Fussion/Serialization/Json.h>
#include <Fussion/Serialization/JsonSerializer.h>
#include <future>
#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>

using namespace Fussion;
namespace fs = std::filesystem;

WorkerPool::WorkerPool()
{
#ifdef FSN_ENABLE_MT_LOADING
    u32 max_threads = std::thread::hardware_concurrency();
#else
    u32 max_threads = 1;
#endif
    LOG_INFOF("Creating {} worker threads for background asset loading.", max_threads);
    m_Quit = false;
    for (u32 i = 0; i < max_threads; i++) {
        m_Workers.emplace_back(&WorkerPool::Work, this, i);
    }
}

WorkerPool::~WorkerPool()
{
    m_Quit = true;
    m_ConditionVariable.notify_all();
    for (auto& worker : m_Workers) {
        worker.join();
    }
}

void WorkerPool::Work(s32 index)
{
    // NOTE: This probably doesn't hurt much since these are pointers to functions, no data are created.
    std::map<AssetType, Ptr<AssetImporter>> asset_serializers {};
    asset_serializers[AssetType::Texture2D] = MakePtr<TextureImporter>();
    asset_serializers[AssetType::Model] = MakePtr<MeshImporter>();

    std::set binaryAssets { AssetType::Model, AssetType::Texture2D };

    auto makeAsset = [](AssetType type) -> Ref<Asset> {
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

            m_ConditionVariable.wait(lock, [this] { return !m_Tasks.empty() || m_Quit; });

            if (!m_Tasks.empty()) {
                task = m_Tasks.front();
                m_Tasks.pop();
            }
        }

        if (task.HasValue()) {
            LOG_INFOF("Worker({}) was notified about a new task: {}", index, task->Path.string());

            auto asset = makeAsset(task->Type);
            auto fullPath = Project::AssetsFolderPath() / task->Path;
            if (binaryAssets.contains(task->Type)) {
                std::ifstream file;
                file.open(fullPath, std::ios::binary | std::ios::in);
                BinaryDeserializer ds(&file);
                asset->Deserialize(ds);
                asset->SetHandle(task->Handle);
                LoadedAssets.Access([&](auto& queue) {
                    queue.push(asset);
                });
            } else {
                if (auto json_string = FileSystem::ReadEntireFile(fullPath)) {
                    YamlDeserializer ds(*json_string);
                    asset->Deserialize(ds);
                    asset->SetHandle(task->Handle);
                    LoadedAssets.Access([&](auto& queue) {
                        queue.push(asset);
                    });
                }
            }
        }

        if (m_Quit) {
            break;
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
    : m_EditorWatcher(FileWatcher::Create(fs::current_path() / "Assets" / "Shaders"))
{
    m_AssetImporters[AssetType::Texture2D] = MakePtr<TextureImporter>();
    m_AssetImporters[AssetType::Model] = MakePtr<MeshImporter>();

    m_EditorWatcher->AddListener([this](fs::path const& path, FileWatcher::EventType type) {
        if (type == FileWatcher::EventType::FileModified) {
            LOG_DEBUGF("Editor file changed: {} type: {}", path.string(), magic_enum::enum_name(type));
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(50ms);
            using namespace std::string_literals;
            auto const full_path = fs::path("Assets") / "Shaders"s / path;
            auto meta = GetMetadata(full_path);
            if (meta.IsEmpty()) {
                return;
            }
            switch (meta->Type) {
            case AssetType::Shader: {
                auto shader = GetAsset(meta->Handle, AssetType::Shader)->As<ShaderAsset>();
                auto result = GPU::ShaderProcessor::CompileSlang(meta->Path);
                if (result) {
                    result->Metadata = shader->GetMetadata();
                    *shader = ShaderAsset(*result, shader->GetColorTargetFormats());
                }
            } break;
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
    // TODO: This is wrong, GetAsset will call functions that will try to lock the registry
    // while we already have it locked.
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

AssetHandle EditorAssetManager::CreateVirtualAsset(Ref<Asset> const& asset, std::string_view name, fs::path const& path)
{
    auto const handle = AssetHandle();
    m_Registry.Access([&](Registry& registry) {
        registry[handle] = EditorAssetMetadata {
            .Type = asset->Type(),
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

bool EditorAssetManager::IsPathAnAsset(fs::path const& path, bool include_virtual) const
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

Maybe<EditorAssetMetadata> EditorAssetManager::GetMetadata(fs::path const& path) const
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

auto metadata_for_asset(AssetType type) -> Ref<AssetMetadata>
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

void EditorAssetManager::RegisterAsset(fs::path const& path, AssetType type)
{
    ZoneScoped;
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
        registry[id] = EditorAssetMetadata {
            .Type = type,
            .Path = path,
            .Name = path.filename().string(),
            .IsVirtual = false,
            .DontSerialize = false,
            .Handle = id,
            .CustomMetadata = metadata_for_asset(type),
        };
    });

    SaveToFile();
}

void EditorAssetManager::ImportAsset(std::filesystem::path const& path, std::filesystem::path const& parentDir)
{
    // 1. Load asset into memory using the appropriate importer (stb_image, tinyglfy, etc..)
    if (!path.has_extension() || !path.has_filename()) {
        LOG_WARNF("Tried importing '{}' which doesn't have an extension. Cannot determine asset type.", path);
        return;
    }

    static auto const FileTypes = std::unordered_map<std::string, AssetType> {
        { ".png", AssetType::Texture2D },
        { ".jpg", AssetType::Texture2D },
        { ".jpeg", AssetType::Texture2D },
        { ".hdr", AssetType::Texture2D },

        { ".glb", AssetType::Model },
        { ".gltf", AssetType::Model },
    };

    auto const ext = path.extension().string();
    if (!FileTypes.contains(ext)) {
        LOG_ERRORF("Do not have importer for this filetype: {}", ext);
        return;
    }
    auto assetType = FileTypes.at(ext);

    auto asset = m_AssetImporters[assetType]->Import(path);

    // 1.1 Run any extra post-processing steps.
    (void)0;

    // 2. Save the asset into a binary form in the project.
    auto name = path.filename();
    name.replace_extension(".fsn");
    auto assetPath = parentDir / name;
    std::ofstream file;
    file.open(assetPath, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        LOG_ERRORF("Could not open file '{}' for writing", assetPath);
        return;
    }

    BinarySerializer s(&file);
    asset->Serialize(s);

    // 3. Register this form in the registry.
    RegisterAsset(relative(assetPath, Project::AssetsFolderPath()), assetType);

    // NOTE: When the asset is loaded, we load that binary format and do not go through the importer.
}

void EditorAssetManager::SaveAsset(AssetHandle handle)
{
    ZoneScoped;
    auto meta = m_Registry.Access([&](Registry& registry) {
        return registry[handle];
    });

    // FIXME: What about binary assets? Should they be able to be saved?
    YamlSerializer ys;
    ys.Initialize();

    m_LoadedAssets[handle]->Serialize(ys);

    auto path = Project::AssetsFolderPath() / meta.Path;
    FileSystem::WriteEntireFile(path, ys.ToString());
}

void EditorAssetManager::RenameAsset(AssetHandle handle, std::string_view new_name)
{
    ZoneScoped;
    m_Registry.Access([&](Registry& registry) {
        if (!registry.contains(handle))
            return;

        auto& meta = registry[handle];
        auto old_path = Project::AssetsFolderPath() / meta.Path;
        auto new_path = Project::AssetsFolderPath() / (meta.Path.has_parent_path() ? meta.Path.parent_path() : "") / new_name;
        LOG_DEBUGF("Renaming asset: '{}' -> '{}'", old_path, new_path);

        if (auto pos = std::ranges::find_if(registry, [&new_path](std::pair<AssetHandle, EditorAssetMetadata> const& pair) {
                return pair.second.Path == relative(new_path, Project::AssetsFolderPath());
            });
            pos != registry.end()) {
            LOG_ERRORF("Rename will overwrite existing asset, aborting.");
            return;
        }

        try {
            fs::rename(old_path, new_path);
            meta.Name = new_name;
            meta.Path = relative(new_path, Project::AssetsFolderPath());
        } catch (fs::filesystem_error& error) {
            LOG_ERRORF("Failed to rename asset: {}", error.what());
        }
    });

    SaveToFile();
}

auto deserialize_custom_metadata(json const& j, AssetType type) -> Ref<AssetMetadata>
{
    ZoneScoped;
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
    YamlSerializer s;
    s.Initialize();

    Serialize(s);
    // s.Write("$Type", "AssetRegistry");
    // // json j = {
    // //     { "$Type", "AssetRegistry" },
    // // };
    //
    // u32 i = 0;
    // m_Registry.Access([&](Registry const& registry) {
    //     s.BeginArray("Assets", registry.size());
    //     for (auto const& [handle, metadata] : registry) {
    //         if (metadata.IsVirtual || metadata.DontSerialize) {
    //             continue;
    //         }
    //
    //         s.BeginObject("", 4);
    //         s.Write("Handle", handle);
    //         s.Write("Type", magic_enum::enum_name(metadata.Type));
    //         s.Write("Path", metadata.Path.string());
    //         s.Write("Name", metadata.Name);
    //         s.EndObject();
    //         // auto index = i++;
    //         // j["Assets"][index] = {
    //         //     { "Handle", handle },
    //         //     { "Type", magic_enum::enum_name(metadata.Type) },
    //         //     { "Path", metadata.Path.string() },
    //         //     { "Name", metadata.Name },
    //         // };
    //
    //         if (metadata.CustomMetadata != nullptr) {
    //             auto ptr = metadata.CustomMetadata->meta_poly_ptr();
    //             auto class_type = ptr.get_type().as_pointer().get_data_type().as_class();
    //             auto name = class_type.get_metadata().at("Name").as<std::string>();
    //
    //             s.Write("CustomMetadata", *metadata.CustomMetadata);
    //
    //             // j["Assets"][index]["CustomMetadata"] = serialize_native_class(class_type, std::move(ptr));
    //             // j["Assets"][index]["CustomMetadata"]["$Type"] = name;
    //         }
    //     }
    //     s.EndArray();
    //
    // });
    FileSystem::WriteEntireFile(Project::AssetRegistryPath(), s.ToString());
}

void EditorAssetManager::LoadFromFile()
{
    ZoneScoped;
    auto const data = FileSystem::ReadEntireFile(Project::AssetRegistryPath());

    YamlDeserializer ds(*data);
    Deserialize(ds);
    // try {
    //     auto j = json::parse(*data);
    //     auto file_type = j["$Type"].get<std::string>();
    //     if (file_type != "AssetRegistry") {
    //         LOG_WARNF("The provided file file is not an AssetRegistry but: {}", file_type);
    //         return;
    //     }
    //
    //     for (auto const& asset : j["Assets"]) {
    //         auto const handle = asset["Handle"].get<Fsn::Uuid>();
    //         auto const type = asset["Type"].get<std::string>();
    //         auto const asset_path = asset["Path"].get<std::string>();
    //         auto name = asset.value("Name", fs::path(asset_path).filename().string());
    //
    //         Uuid h { handle };
    //
    //         m_Registry.Access([&](Registry& registry) {
    //             registry[h] = EditorAssetMetadata {
    //                 .Type = *magic_enum::enum_cast<AssetType>(type),
    //                 .Path = asset_path,
    //                 .Name = name,
    //                 .Handle = h,
    //             };
    //
    //             registry[h].CustomMetadata = deserialize_custom_metadata(asset, registry[h].Type);
    //         });
    //     }
    // } catch (std::exception const& e) {
    //     LOG_ERRORF("Exception caught while deserialize asset registry: {}", e.what());
    // }
}

void EditorAssetManager::RefreshAsset(AssetHandle handle)
{
    if (!IsAssetHandleValid(handle) || !IsAssetLoaded(handle))
        return;

    SaveAsset(handle);
}

void EditorAssetManager::MoveAsset(AssetHandle handle, fs::path const& path)
{
    m_Registry.Access([&](Registry& registry) {
        if (!registry.contains(handle) || !fs::is_directory(path) || !fs::exists(path)) {
            return;
        }

        auto& meta = registry[handle];

        auto filename = meta.Path.filename();
        auto new_path = Project::AssetsFolderPath() / path / filename;
        LOG_INFOF("Moving asset '{}' -> '{}'", filename, new_path);

        try {
            fs::rename(Project::AssetsFolderPath() / meta.Path, new_path);
            meta.Path = relative(new_path, Project::AssetsFolderPath());
        } catch (fs::filesystem_error& error) {
            LOG_ERRORF("Failed to move asset: {}", error.what());
        }
    });

    SaveToFile();
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

    m_Registry.Access([&](Registry const& registry) {
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

            EditorAssetMetadata metadata {};
            ctx.Read("Name", metadata.Name);
            ctx.Read("Handle", metadata.Handle);
            ctx.Read("Type", metadata.Type);
            ctx.Read("Path", metadata.Path);

            switch (metadata.Type) {
            case AssetType::Texture2D: {
                auto meta = MakeRef<Texture2DMetadata>();
                ctx.Read("CustomMetadata", *meta);
                metadata.CustomMetadata = meta;
            } break;
            default:
                break;
            }

            registry[metadata.Handle] = metadata;

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
