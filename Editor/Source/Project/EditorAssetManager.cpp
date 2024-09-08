#include "EditorPCH.h"
#include "EditorAssetManager.h"
#include "EditorApplication.h"
#include "Project.h"
#include "Fussion/Assets/Model.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Serialization/AssetSerializer.h"
#include "Serialization/TextureSerializer.h"
#include "Serialization/MeshSerializer.h"

#include <Fussion/OS/FileSystem.h>
#include <Fussion/Serialization/Json.h>
#include <Fussion/Serialization/JsonSerializer.h>
#include <Fussion/Scene/Scene.h>
#include <Fussion/Assets/ShaderAsset.h>

#include <magic_enum/magic_enum.hpp>
#include <tracy/Tracy.hpp>
#include <future>

using namespace Fussion;
namespace fs = std::filesystem;

WorkerPool::WorkerPool(EditorAssetManager* asset_manager): m_asset_manager(asset_manager)
{
    auto max_threads = std::thread::hardware_concurrency();
    // auto max_threads = 1;
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
    std::map<AssetType, Ptr<AssetSerializer>> asset_serializers{};
    asset_serializers[AssetType::Texture2D] = make_ptr<TextureSerializer>();
    asset_serializers[AssetType::Model] = make_ptr<MeshSerializer>();

    auto make_asset = [](AssetType type) -> Ref<Asset> {
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
            UNREACHABLE;
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

            if (asset_serializers.contains(task->type)) {
                auto asset = asset_serializers[task->type]->Load(*task);
                asset->set_handle(task->handle);
                loaded_assets.access([&](auto& queue) {
                    queue.push(asset);
                });
            } else {
                auto asset = make_asset(task->type);
                if (auto json_string = FileSystem::read_entire_file(Project::assets_folder() / task->path)) {
                    JsonDeserializer ds(*json_string);
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

void WorkerPool::load(EditorAssetMetadata const& metadata)
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
    m_asset_importers[AssetType::Texture2D] = make_ptr<TextureSerializer>();
    m_asset_importers[AssetType::Model] = make_ptr<MeshSerializer>();

    m_editor_watcher->register_listener([this](fs::path const& path, FileWatcher::EventType type) {
        LOG_DEBUGF("Editor file changed: {} type: {}", path.string(), magic_enum::enum_name(type));
        if (type == FileWatcher::EventType::FileModified) {
            using namespace std::chrono_literals;

            std::this_thread::sleep_for(100ms);
            using namespace std::string_literals;
            auto full_path = fs::path("Assets") / "Shaders"s / path;
            auto meta = get_metadata(full_path);
            if (meta.is_empty()) {
                return;
            }
            switch (meta->type) {
            case AssetType::Shader: {
                // auto shader = get_asset(meta->handle, AssetType::Shader)->as<ShaderAsset>();
                // auto data = FileSystem::read_entire_file(meta->path);
                // auto result = RHI::ShaderCompiler::Compile(*data);
                // if (result.has_value()) {
                //     *shader = ShaderAsset(shader->AssociatedRenderPass(), result->ShaderStages, result->Metadata);
                // }
            }
            break;
            default:
                break;
            }
        }
    });
    m_editor_watcher->start();
}

EditorAssetManager::~EditorAssetManager() = default;

Asset* EditorAssetManager::get_asset(AssetHandle handle, AssetType type)
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

auto EditorAssetManager::get_asset(std::string const& path, AssetType type) -> Asset*
{
    return m_registry.access([&](Registry const& registry) -> Asset* {
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

AssetHandle EditorAssetManager::create_virtual_asset(Ref<Asset> const& asset, std::string_view name, fs::path const& path)
{
    auto handle = AssetHandle();
    m_registry.access([&](Registry& registry) {
        registry[handle] = EditorAssetMetadata{
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

auto MetadataForAsset(AssetType type) -> Ref<AssetMetadata>
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

void EditorAssetManager::register_asset(fs::path const& path, AssetType type)
{
    if (type == AssetType::Invalid) {
        LOG_WARNF("Ignoring Invalid asset type.");
        return;
    }
    m_registry.access([&](Registry& registry) {
        auto pos = std::ranges::find_if(registry, [&path](auto entry) -> bool { return entry.second.path == path; });
        if (pos != registry.end()) {
            LOG_ERRORF("Cannot register asset at path '{}', another asset lives there", path.string());
            return;
        }

        LOG_INFOF("Registering '{}' of type '{}'", path.string(), magic_enum::enum_name(type));

        Uuid id;
        registry[id] = EditorAssetMetadata{
            .type = type,
            .path = path,
            .name = path.filename().string(),
            .is_virtual = false,
            .dont_serialize = false,
            .handle = id,
            .custom_metadata = MetadataForAsset(type),
        };
    });

    save_to_file();
}

void EditorAssetManager::save_asset(AssetHandle handle)
{
    auto meta = m_registry.access([&](Registry& registry) {
        return registry[handle];
    });

    if (m_asset_importers.contains(meta.type)) {
        m_asset_importers[meta.type]->Save(meta, m_loaded_assets[handle]);
        m_loaded_assets[handle] = m_asset_importers[meta.type]->Load(meta);
        m_loaded_assets[handle]->set_handle(handle);
    } else {
        JsonSerializer js;
        js.initialize();

        m_loaded_assets[handle]->serialize(js);

        auto path = Project::assets_folder() / meta.path;
        FileSystem::write_entire_file(path, js.to_string());
    }
}

void EditorAssetManager::save_asset(Ref<Asset> const& asset)
{
    auto const handle = asset->handle();
    m_registry.access([&](Registry& registry) {
        m_asset_importers[registry[handle].type]->Save(registry[handle], asset);

        m_loaded_assets[handle] = m_asset_importers[registry[handle].type]->Load(registry[handle]);
        m_loaded_assets[handle]->set_handle(handle);
    });
}

void EditorAssetManager::rename_asset(AssetHandle handle, std::string_view new_name)
{
    m_registry.access([&](Registry& registry) {
        if (!registry.contains(handle))
            return;

        auto& meta = registry[handle];
        auto old_path = Project::assets_folder() / meta.path;
        auto new_path = Project::assets_folder() / (meta.path.has_parent_path() ? meta.path.parent_path() : "") / new_name;
        LOG_DEBUGF("Renaming asset: '{}' -> '{}'", old_path, new_path);

        if (auto pos = std::ranges::find_if(registry, [&new_path](std::pair<AssetHandle, EditorAssetMetadata> const& pair) {
            return pair.second.path == relative(new_path, Project::assets_folder());
        }); pos != registry.end()) {
            LOG_ERRORF("Rename will overwrite existing asset, aborting.");
            return;
        }

        try {
            fs::rename(old_path, new_path);
            meta.name = new_name;
            meta.path = relative(new_path, Project::assets_folder());
        } catch (fs::filesystem_error& error) {
            LOG_ERRORF("Failed to rename asset: {}", error.what());
        }
    });

    save_to_file();
}

auto DeserializeCustomMetadata(json const& j, AssetType type) -> Ref<AssetMetadata>
{
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
    json j = {
        { "$Type", "AssetRegistry" },
    };

    u32 i = 0;
    m_registry.access([&](Registry const& registry) {
        for (auto const& [handle, metadata] : registry) {
            if (metadata.is_virtual || metadata.dont_serialize) {
                continue;
            }

            auto index = i++;
            j["Assets"][index] = {
                { "Handle", handle },
                { "Type", magic_enum::enum_name(metadata.type) },
                { "Path", metadata.path.string() },
                { "Name", metadata.name },
            };

            if (metadata.custom_metadata != nullptr) {
                auto ptr = metadata.custom_metadata->meta_poly_ptr();
                auto class_type = ptr.get_type().as_pointer().get_data_type().as_class();
                auto name = class_type.get_metadata().at("Name").as<std::string>();
                j["Assets"][index]["CustomMetadata"] = serialize_native_class(class_type, std::move(ptr));
                j["Assets"][index]["CustomMetadata"]["$Type"] = name;
            }
        }

        FileSystem::write_entire_file(Project::asset_registry(), j.dump(2));
    });
}

void EditorAssetManager::load_from_file()
{
    ZoneScoped;
    auto const data = FileSystem::read_entire_file(Project::asset_registry());

    // JsonDeserializer ds(*data);
    // Deserialize(ds);
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
            auto name = asset.value("Name", fs::path(asset_path).filename().string());

            Uuid h{ handle };

            m_registry.access([&](Registry& registry) {
                registry[h] = EditorAssetMetadata{
                    .type = *magic_enum::enum_cast<AssetType>(type),
                    .path = asset_path,
                    .name = name,
                    .handle = h,
                };

                registry[h].custom_metadata = DeserializeCustomMetadata(asset, registry[h].type);
            });
        }
    } catch (std::exception const& e) {
        LOG_ERRORF("Exception caught while deserialize asset registry: {}", e.what());
    }
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
        auto new_path = Project::assets_folder() / path / filename;
        LOG_INFOF("Moving asset '{}' -> '{}'", filename, new_path);

        try {
            fs::rename(Project::assets_folder() / meta.path, new_path);
            meta.path = relative(new_path, Project::assets_folder());
        } catch (fs::filesystem_error& error) {
            LOG_ERRORF("Failed to move asset: {}", error.what());
        }
    });

    save_to_file();
}

void EditorAssetManager::check_for_loaded_assets()
{
    m_worker_pool.loaded_assets.access([this](std::queue<Ref<Asset>>& queue) {
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

            EditorAssetMetadata metadata{};
            ctx.read("Name", metadata.name);
            ctx.read("Handle", metadata.handle);
            ctx.read("Type", metadata.type);
            ctx.read("Path", metadata.path);

            switch (metadata.type) {
            case AssetType::Texture2D: {
                auto meta = make_ref<Texture2DMetadata>();
                ctx.read("CustomMetadata", *meta);
                metadata.custom_metadata = meta;
            }
            break;
            default:
                break;
            }

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
        m_worker_pool.load(registry[handle]);
#endif

    });

}
