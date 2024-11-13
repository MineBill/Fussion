#pragma once
#include "../Serialization/AssetImporter.h"
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

    void load_asset(EditorAssetMetadata const& metadata);

    Fussion::ThreadProtected<std::queue<Ref<Fussion::AssetBase>>> loaded_assets {};

private:
    void work(s32 index);

    std::queue<EditorAssetMetadata> m_tasks {};

    std::mutex m_mutex {};
    std::vector<std::thread> m_workers {};
    std::condition_variable m_condition_variable {};
    std::atomic_bool m_quit {};
};

class AssetImporter;

class EditorAssetManager final : public Fussion::AssetManagerBase
    , Fussion::ISerializable {
    // NOTE: Is there a better way/abstraction to expose internals to the worker pool?
    friend WorkerPool;

public:
    using Registry = std::unordered_map<Fsn::AssetHandle, EditorAssetMetadata>;

    EditorAssetManager();
    virtual ~EditorAssetManager() override;

    virtual auto get_asset(Fsn::AssetHandle handle, Fsn::AssetType type) -> Fussion::AssetBase* override;
    virtual auto get_asset(std::string const& path, Fussion::AssetType type) -> Fussion::AssetBase* override;

    virtual bool is_asset_loaded(Fsn::AssetHandle handle) override;
    virtual bool is_asset_handle_valid(Fsn::AssetHandle handle) const override;
    virtual bool is_asset_virtual(Fussion::AssetHandle handle) override;
    virtual auto create_virtual_asset(Ref<Fussion::AssetBase> const& asset, std::string_view name, std::filesystem::path const& path) -> Fussion::AssetHandle override;
    virtual auto get_asset_metadata(Fussion::AssetHandle handle) -> Fussion::AssetMetadata* override;

    bool is_asset_loading(Fussion::AssetHandle handle);

    bool is_path_an_asset(std::filesystem::path const& path, bool include_virtual = false) const;
    auto get_metadata(std::filesystem::path const& path) const -> Maybe<EditorAssetMetadata>;
    auto get_metadata(Fsn::AssetHandle handle) const -> EditorAssetMetadata;
    /// @param parent_dir The parent directory to place the binary asset inside.
    void import_asset(std::filesystem::path const& path, std::filesystem::path const& parent_dir);

    auto registry() -> Fussion::ThreadProtected<Registry>& { return m_registry; }

    void register_asset(std::string_view name, std::filesystem::path const& path, Ref<Fsn::AssetBase> const& asset);
    void save_asset(Fussion::AssetHandle handle);
    void rename_asset(Fussion::AssetHandle handle, std::string_view new_name);
    void move_asset(Fussion::AssetHandle handle, std::filesystem::path const& path);
    void refresh_asset(Fussion::AssetHandle handle);

    /// Creates a new asset of type T.
    /// @tparam T
    /// @param name The name of the asset. This should not include the extension.
    /// @param path The containing folder where the asset should be created into, relative to the assets folder root.
    /// @return
    template<std::derived_from<Fsn::AssetBase> T>
    auto create_asset(std::string_view name, std::filesystem::path const& path) -> Ref<Fsn::AssetBase>
    {
        auto asset = make_ref<T>();
        register_asset(name, path, asset);
        return asset;
    }

    void save_to_file();
    void load_from_file();

    // NOTE: I'm not sure if this is a good way to go about it. Could some kind of callback
    // be used instead?
    void check_for_loaded_assets();

private:
    virtual void serialize(Fussion::Serializer& ctx) const override;
    virtual void deserialize(Fussion::Deserializer& ctx) override;

    void load_asset(Fussion::AssetHandle handle, Fussion::AssetType type);

    Fussion::ThreadProtected<Registry> m_registry {};
    std::unordered_map<Fsn::AssetHandle, Ref<Fsn::AssetBase>> m_loaded_assets {};

    std::unordered_map<Fsn::AssetType, Ptr<AssetImporter>> m_asset_importers {};

    Ptr<Fussion::FileWatcher> m_editor_watcher {};

    WorkerPool m_worker_pool {};
};
