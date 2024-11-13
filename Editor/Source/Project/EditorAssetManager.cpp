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
    m_quit = false;
    for (u32 i = 0; i < max_threads; i++) {
        m_workers.emplace_back(&WorkerPool::work, this, i);
    }
}

WorkerPool::~WorkerPool()
{
    m_quit = true;
    m_condition_variable.notify_all();
    for (auto& worker : m_workers) {
        worker.join();
    }
}

void WorkerPool::work(s32 index)
{
    // NOTE: This probably doesn't hurt much since these are pointers to functions, no data are created.
    std::map<AssetType, Ptr<AssetImporter>> asset_serializers {};
    asset_serializers[AssetType::Texture2D] = make_ptr<TextureImporter>();
    asset_serializers[AssetType::Model] = make_ptr<MeshImporter>();

    std::set binaryAssets { AssetType::Model, AssetType::Texture2D };

    auto make_asset = [](AssetType type) -> Ref<AssetBase> {
        switch (type) {
        case AssetType::Model:
            return make_ref<Model>();
        case AssetType::PbrMaterial:
            return make_ref<PbrMaterial>();
        case AssetType::Scene:
            return make_ref<Scene>();
        case AssetType::Texture2D:
            return make_ref<Texture2D>();
        default:
            UNREACHABLE();
        }
    };

    LOG_INFOF("Worker({}) started", index);

    while (true) {
        Maybe<EditorAssetMetadata> task;
        {
            std::unique_lock lock(m_mutex);

            m_condition_variable.wait(lock, [this] { return !m_tasks.empty() || m_quit; });

            if (!m_tasks.empty()) {
                task = m_tasks.front();
                m_tasks.pop();
            }
        }

        if (task.has_value()) {
            LOG_INFOF("Worker({}) was notified about a new task: {}", index, task->path.string());

            auto asset_base = make_asset(task->type);
            auto fullPath = Project::assets_folder_path() / task->path;
            if (auto binary = std::dynamic_pointer_cast<BinaryAsset>(asset_base)) {
                std::ifstream file;
                file.open(fullPath, std::ios::binary | std::ios::in);
                binary->deserialize(file);
                binary->set_handle(task->handle);
                loaded_assets.access([&](auto& queue) {
                    queue.push(binary);
                });
            } else {
                if (auto json_string = FileSystem::read_entire_file(fullPath)) {
                    YamlDeserializer ds(*json_string);
                    auto asset = asset_base->as<Asset>();
                    asset->deserialize(ds);
                    asset->set_handle(task->handle);
                    loaded_assets.access([&](auto& queue) {
                        queue.push(asset);
                    });
                }
            }
        }

        if (m_quit) {
            break;
        }
    }
}

void WorkerPool::load_asset(EditorAssetMetadata const& metadata)
{
    {
        std::lock_guard lock(m_mutex);
        m_tasks.push(metadata);
    }

    m_condition_variable.notify_one();
}

EditorAssetManager::EditorAssetManager()
    : m_editor_watcher(FileWatcher::create(fs::current_path() / "Assets" / "Shaders"))
{
    m_asset_importers[AssetType::Texture2D] = make_ptr<TextureImporter>();
    m_asset_importers[AssetType::Model] = make_ptr<MeshImporter>();

    m_editor_watcher->add_listener([this](fs::path const& path, FileWatcher::EventType type) {
        if (type == FileWatcher::EventType::FileModified) {
            LOG_DEBUGF("Editor file changed: {} type: {}", path.string(), magic_enum::enum_name(type));
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(50ms);
            using namespace std::string_literals;
            auto const full_path = fs::path("Assets") / "Shaders"s / path;
            auto meta = get_metadata(full_path);
            if (meta.is_empty()) {
                return;
            }
            switch (meta->type) {
            case AssetType::Shader: {
                auto shader = get_asset(meta->handle, AssetType::Shader)->as<ShaderAsset>();
                auto result = GPU::ShaderProcessor::compile_slang(meta->path);
                if (result) {
                    result->metadata = shader->metadata();
                    *shader = ShaderAsset(*result, shader->color_target_formats());
                }
            } break;
            default:
                break;
            }
        }
    });
    m_editor_watcher->start();
}

EditorAssetManager::~EditorAssetManager() = default;

auto EditorAssetManager::get_asset(AssetHandle handle, AssetType type) -> AssetBase*
{
    // VERIFY(m_Registry.contains(handle), "The registry does not contain this asset handle: {}. Could it be that you are referencing a virtual asset?", handle);
    ZoneScoped;
    if (!is_asset_loaded(handle)) {
        if (is_asset_loading(handle)) {
            return nullptr;
        }
        load_asset(handle, type);
        return nullptr;
    }
    return m_loaded_assets[handle].get();
}

auto EditorAssetManager::get_asset(std::string const& path, AssetType type) -> AssetBase*
{
    // TODO: This is wrong, GetAsset will call functions that will try to lock the registry
    // while we already have it locked.
    return m_registry.access([&](Registry const& registry) -> AssetBase* {
        for (auto const& [handle, asset] : registry) {
            if (asset.path == path && asset.type == type) {
                return get_asset(handle, type);
            }
        }
        return nullptr;
    });
}

bool EditorAssetManager::is_asset_loaded(AssetHandle handle)
{
    check_for_loaded_assets();
    return m_registry.access([&](Registry const& registry) {
        auto const& metadata = registry.at(handle);
        return metadata.load_state == AssetLoadState::Loaded;
    });
}

bool EditorAssetManager::is_asset_handle_valid(AssetHandle handle) const
{
    return m_registry.access([&](Registry const& registry) {
        return registry.contains(handle);
    });
}

bool EditorAssetManager::is_asset_virtual(AssetHandle handle)
{
    return m_registry.access([&](Registry const& registry) {
        if (!registry.contains(handle))
            return false;
        return registry.at(handle).is_virtual;
    });
}

AssetHandle EditorAssetManager::create_virtual_asset(Ref<AssetBase> const& asset, std::string_view name, fs::path const& path)
{
    auto const handle = AssetHandle();
    m_registry.access([&](Registry& registry) {
        registry[handle] = EditorAssetMetadata {
            .type = asset->type(),
            .path = path,
            .name = std::string(name),
            .is_virtual = true,
            .dont_serialize = true,
            .load_state = AssetLoadState::Loaded,
            .handle = handle,
        };
    });
    m_loaded_assets[handle] = asset;

    return handle;
}

AssetMetadata* EditorAssetManager::get_asset_metadata(AssetHandle handle)
{
    if (!is_asset_handle_valid(handle))
        return nullptr;

    return m_registry.access([&](Registry& registry) {
        return registry[handle].custom_metadata.get();
    });
}

bool EditorAssetManager::is_asset_loading(AssetHandle handle)
{
    return m_registry.access([&](Registry const& registry) {
        auto const& metadata = registry.at(handle);
        return metadata.load_state == AssetLoadState::Loading;
    });
}

bool EditorAssetManager::is_path_an_asset(fs::path const& path, bool include_virtual) const
{
    ZoneScoped;
    return m_registry.access([&](Registry const& registry) {
        for (auto const& [id, metadata] : registry) {
            (void)id;
            if (!include_virtual && metadata.is_virtual)
                continue;
            if (metadata.path == path) {
                return true;
            }
        }
        return false;
    });
}

Maybe<EditorAssetMetadata> EditorAssetManager::get_metadata(fs::path const& path) const
{
    ZoneScoped;
    return m_registry.access([&](Registry const& registry) -> EditorAssetMetadata {
        for (auto const& [id, metadata] : registry) {
            (void)id;
            if (metadata.path == path) {
                return metadata;
            }
        }
        return {};
    });
}

EditorAssetMetadata EditorAssetManager::get_metadata(AssetHandle handle) const
{
    if (is_asset_handle_valid(handle)) {
        return m_registry.access([&](Registry const& registry) {
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
        return make_ref<Texture2DMetadata>();
    }
    default:
        break;
    }
    return nullptr;
}

// void EditorAssetManager::register_asset(fs::path const& path, AssetType type)
// {
//     ZoneScoped;
//     if (type == AssetType::Invalid) {
//         LOG_WARNF("Ignoring Invalid asset type.");
//         return;
//     }
//     m_registry.access([&](Registry& registry) {
//         auto pos = std::ranges::find_if(registry, [&path](auto entry) -> bool { return entry.second.path == path; });
//         if (pos != registry.end()) {
//             LOG_ERRORF("Cannot register asset at path '{}', another asset lives there", path.string());
//             return;
//         }
//
//         LOG_INFOF("Registering '{}' of type '{}'", path.string(), magic_enum::enum_name(type));
//
//         Uuid id;
//         registry[id] = EditorAssetMetadata {
//             .type = type,
//             .path = path,
//             .name = path.filename().string(),
//             .is_virtual = false,
//             .dont_serialize = false,
//             .handle = id,
//             .custom_metadata = metadata_for_asset(type),
//         };
//     });
//
//     save_to_file();
// }

void EditorAssetManager::import_asset(std::filesystem::path const& path, std::filesystem::path const& parent_dir)
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

    auto ext = path.extension().string();
    if (!FileTypes.contains(ext)) {
        LOG_ERRORF("Do not have importer for this filetype: {}", ext);
        return;
    }
    auto asset_type = FileTypes.at(ext);

    auto asset = m_asset_importers[asset_type]->import(path);

    // 1.1 Run any extra post-processing steps.
    (void)0;

    // 2. Save the asset into a binary form in the project.
    auto name = path.filename();
    name.replace_extension(".fsn");

    // 3. Register this form in the registry.
    register_asset(name.string(), relative(parent_dir, Project::assets_folder_path()), asset);

    // NOTE: When the asset is loaded, we load that binary format and do not go through the importer.
}

void EditorAssetManager::register_asset(std::string_view name, std::filesystem::path const& path, Ref<Fsn::AssetBase> const& asset)
{
    if (name.empty()) {
        LOG_ERRORF("Invalid name: '{}'", name);
        return;
    }

    auto normal_path = (path / (std::string(name) + ".fsn")).lexically_normal();
    Fussion::AssetHandle handle;

    m_registry.access([&](auto& registry) {
        registry[handle] = EditorAssetMetadata {
            .type = asset->type(),
            .path = normal_path,
            .name = std::string(name),
            .is_virtual = false,
            .dont_serialize = false,
            .handle = handle,
        };
    });

    m_loaded_assets[handle] = asset;

    // Create the necessary directories, recursively.
    auto base_path = normal_path;
    base_path.remove_filename();
    if (!base_path.empty()) {
        try {
            std::filesystem::create_directories(base_path);
        } catch (std::exception& e) {
            LOG_DEBUGF("Exception caught in create_directories: '{}'\npath {}", e.what(), normal_path.string());
        }
    }

    save_asset(handle);

    save_to_file();
}

void EditorAssetManager:: save_asset(AssetHandle handle)
{
    ZoneScoped;
    auto meta = m_registry.access([&](Registry& registry) {
        return registry[handle];
    });

    if (auto binary = std::dynamic_pointer_cast<BinaryAsset>(m_loaded_assets[handle])) {
        std::ofstream file;
        file.open(Project::assets_folder_path() / meta.path, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            LOG_ERRORF("Could not open file '{}' for writing", meta.path);
            return;
        }
        binary->serialize(file);
    } else {
        YamlSerializer ys;
        ys.initialize();

        m_loaded_assets[handle]->as<Asset>()->serialize(ys);

        auto path = Project::assets_folder_path() / meta.path;
        FileSystem::write_entire_file(path, ys.to_string());
    }
}

void EditorAssetManager::rename_asset(AssetHandle handle, std::string_view new_name)
{
    ZoneScoped;
    m_registry.access([&](Registry& registry) {
        if (!registry.contains(handle))
            return;

        auto& meta = registry[handle];
        auto old_path = Project::assets_folder_path() / meta.path;
        auto new_path = Project::assets_folder_path() / (meta.path.has_parent_path() ? meta.path.parent_path() : "") / new_name;
        LOG_DEBUGF("Renaming asset: '{}' -> '{}'", old_path, new_path);

        if (auto pos = std::ranges::find_if(registry, [&new_path](std::pair<AssetHandle, EditorAssetMetadata> const& pair) {
                return pair.second.path == relative(new_path, Project::assets_folder_path());
            });
            pos != registry.end()) {
            LOG_ERRORF("Rename will overwrite existing asset, aborting.");
            return;
        }

        try {
            fs::rename(old_path, new_path);
            meta.name = new_name;
            meta.path = relative(new_path, Project::assets_folder_path());
        } catch (fs::filesystem_error& error) {
            LOG_ERRORF("Failed to rename asset: {}", error.what());
        }
    });

    save_to_file();
}

auto deserialize_custom_metadata(json const& j, AssetType type) -> Ref<AssetMetadata>
{
    ZoneScoped;
    using enum AssetType;
    switch (type) {
    case Texture2D: {
        auto meta = make_ref<Texture2DMetadata>();
        JsonDeserializer ds = JsonDeserializer::from_json_object(j);

        ds.read("CustomMetadata", *meta);
        return meta;
    }
    default:
        break;
    }
    return nullptr;
}

void EditorAssetManager::save_to_file()
{
    ZoneScoped;
    YamlSerializer s;
    s.initialize();

    serialize(s);
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
    FileSystem::write_entire_file(Project::asset_registry_path(), s.to_string());
}

void EditorAssetManager::load_from_file()
{
    ZoneScoped;
    auto const data = FileSystem::read_entire_file(Project::asset_registry_path());

    YamlDeserializer ds(*data);
    deserialize(ds);
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

void EditorAssetManager::refresh_asset(AssetHandle handle)
{
    if (!is_asset_handle_valid(handle) || !is_asset_loaded(handle))
        return;

    save_asset(handle);
}

void EditorAssetManager::move_asset(AssetHandle handle, fs::path const& path)
{
    m_registry.access([&](Registry& registry) {
        if (!registry.contains(handle) || !fs::is_directory(path) || !fs::exists(path)) {
            return;
        }

        auto& meta = registry[handle];

        auto filename = meta.path.filename();
        auto new_path = Project::assets_folder_path() / path / filename;
        LOG_INFOF("Moving asset '{}' -> '{}'", filename, new_path);

        try {
            fs::rename(Project::assets_folder_path() / meta.path, new_path);
            meta.path = relative(new_path, Project::assets_folder_path());
        } catch (fs::filesystem_error& error) {
            LOG_ERRORF("Failed to move asset: {}", error.what());
        }
    });

    save_to_file();
}

void EditorAssetManager::check_for_loaded_assets()
{
    m_worker_pool.loaded_assets.access([this](std::queue<Ref<AssetBase>>& queue) {
        while (!queue.empty()) {
            auto asset = queue.front();
            queue.pop();

            auto handle = asset->handle();
            m_loaded_assets[handle] = asset;

            m_registry.access([&](Registry& registry) {
                registry[handle].load_state = AssetLoadState::Loaded;
            });
        }
    });
}

void EditorAssetManager::serialize(Serializer& ctx) const
{
    ISerializable::serialize(ctx);

    m_registry.access([&](Registry const& registry) {
        ctx.begin_array("Assets", registry.size());
        for (auto const& metadata : registry | std::views::values) {
            if (metadata.is_virtual || metadata.dont_serialize)
                continue;
            ctx.begin_object("", 0);
            ctx.write("Name", metadata.name);
            ctx.write("Handle", metadata.handle);
            ctx.write("Type", metadata.type);
            ctx.write("Path", metadata.path);

            if (metadata.custom_metadata != nullptr) {
                ctx.write("CustomMetadata", *metadata.custom_metadata);
            }
            ctx.end_object();
        }
        ctx.end_array();
    });
}

void EditorAssetManager::deserialize(Deserializer& ctx)
{
    ISerializable::deserialize(ctx);
    size_t size;
    ctx.begin_array("Assets", size);
    m_registry.access([&](Registry& registry) {
        registry.reserve(size);

        for (size_t i = 0; i < size; i++) {
            size_t _s;
            ctx.begin_object("", _s);

            EditorAssetMetadata metadata {};
            ctx.read("Name", metadata.name);
            ctx.read("Handle", metadata.handle);
            ctx.read("Type", metadata.type);
            ctx.read("Path", metadata.path);

            registry[metadata.handle] = metadata;

            ctx.end_object();
        }
    });
    ctx.end_array();
}

void EditorAssetManager::load_asset(AssetHandle handle, AssetType type)
{
    LOG_DEBUGF("LoadAsset[{}, {}]", handle, magic_enum::enum_name(type));
    m_registry.access([&](Registry& registry) {
        registry[handle].load_state = AssetLoadState::Loading;
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
        m_worker_pool.load_asset(registry[handle]);
#endif
    });
}
