#pragma once
#include "../Serialization/AssetSerializer.h"
#include "EditorAssetMetadata.h"

#include "Fussion/Assets/Asset.h"
#include "Fussion/Assets/AssetManagerBase.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Core/Maybe.h"
#include "Fussion/Core/ThreadProtected.h"
#include "Fussion/Core/Types.h"
#include "Fussion/OS/FileWatcher.h"

#include <condition_variable>
#include <filesystem>
#include <queue>
#include <unordered_map>

class EditorAssetManager;

class WorkerPool final {
public:
    explicit WorkerPool();
    ~WorkerPool();

    void load(EditorAssetMetadata const& metadata);

    Fussion::ThreadProtected<std::queue<Ref<Fussion::Asset>>> loaded_assets{};

private:
    void work(s32 index);

    std::queue<EditorAssetMetadata> m_tasks{};

    std::mutex m_mutex{};
    std::vector<std::thread> m_workers{};
    std::condition_variable m_condition_variable{};
    std::atomic_bool m_quit{};
};

class AssetSerializer;

class EditorAssetManager final : public Fussion::AssetManagerBase, Fussion::ISerializable {
    // NOTE: Is there a better way/abstraction to expose internals to the worker pool?
    friend WorkerPool;

public:
    using Registry = std::unordered_map<Fsn::AssetHandle, EditorAssetMetadata>;

    EditorAssetManager();
    virtual ~EditorAssetManager() override;

    virtual auto get_asset(Fsn::AssetHandle handle, Fsn::AssetType type) -> Fussion::Asset* override;
    virtual auto get_asset(std::string const& path, Fussion::AssetType type) -> Fussion::Asset* override;

    virtual bool is_asset_loaded(Fsn::AssetHandle handle) override;
    virtual bool is_asset_handle_valid(Fsn::AssetHandle handle) const override;
    virtual bool is_asset_virtual(Fussion::AssetHandle handle) override;
    virtual auto create_virtual_asset(Ref<Fussion::Asset> const& asset, std::string_view name, std::filesystem::path const& path) -> Fussion::AssetHandle override;
    virtual auto get_asset_metadata(Fussion::AssetHandle handle) -> Fussion::AssetMetadata* override;

    bool is_asset_loading(Fussion::AssetHandle handle);

    bool is_path_an_asset(std::filesystem::path const& path, bool include_virtual = false) const;
    auto get_metadata(std::filesystem::path const& path) const -> Maybe<EditorAssetMetadata>;
    auto get_metadata(Fsn::AssetHandle handle) const -> EditorAssetMetadata;
    void register_asset(std::filesystem::path const& path, Fussion::AssetType type);

    auto registry() -> Fussion::ThreadProtected<Registry>& { return m_registry; }

    template<std::derived_from<Fsn::Asset> T>
    auto create_asset(std::filesystem::path const& path) -> Fsn::AssetRef<T>
    {
        auto normal_path = path.lexically_normal();
        Fussion::AssetHandle handle;

        m_registry.access([&](auto& registry) {
            registry[handle] = EditorAssetMetadata{
                .type = T::static_type(),
                .path = normal_path,
                .name = path.filename().string(),
                .is_virtual = false,
                .dont_serialize = false,
                .handle = handle,
            };
        });

        m_loaded_assets[handle] = make_ref<T>();

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

        return Fsn::AssetRef<T>(handle);
    }

    void save_asset(Fussion::AssetHandle handle);
    void save_asset(Ref<Fussion::Asset> const& asset);

    void rename_asset(Fussion::AssetHandle handle, std::string_view new_name);
    void move_asset(Fussion::AssetHandle handle, std::filesystem::path const& path);

    void save_to_file();
    void load_from_file();
    void refresh_asset(Fussion::AssetHandle handle);

    // NOTE: I'm not sure if this is a good way to go about it. Could some kind of callback
    // be used instead?
    void check_for_loaded_assets();

private:
    virtual void serialize(Fussion::Serializer& ctx) const override;
    virtual void deserialize(Fussion::Deserializer& ctx) override;

    void load_asset(Fussion::AssetHandle handle, Fussion::AssetType type);

    Fussion::ThreadProtected<Registry> m_registry{};
    std::unordered_map<Fsn::AssetHandle, Ref<Fsn::Asset>> m_loaded_assets{};

    std::unordered_map<Fsn::AssetType, Ptr<AssetSerializer>> m_asset_importers{};

    Ptr<Fussion::FileWatcher> m_editor_watcher{};

    WorkerPool m_worker_pool{};
};
