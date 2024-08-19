#pragma once
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

enum class AssetLoadState {
    Unloaded = 0,
    Loading,
    Loaded,
    FailedToLoad,
};

struct EditorAssetMetadata final {
    Fsn::AssetType Type = Fsn::AssetType::Invalid;
    std::filesystem::path Path;
    std::string Name;

    bool IsVirtual = false;
    bool DontSerialize = false;
    bool Dirty = false;

    AssetLoadState LoadState{ AssetLoadState::Unloaded };

    Fussion::AssetHandle Handle;

    // TODO: Investigate if using Ref is a good idea.
    Ref<Fussion::AssetMetadata> CustomMetadata{ nullptr };
    bool IsValid() const { return Type != Fsn::AssetType::Invalid; }
};

class WorkerPool final {
public:
    explicit WorkerPool(EditorAssetManager* asset_manager);

    void Load(EditorAssetMetadata const& metadata);

    Fussion::ThreadProtected<std::queue<Ref<Fussion::Asset>>> LoadedAssets{};
private:
    void Work(s32 index);

    std::queue<EditorAssetMetadata> m_Tasks{};

    std::mutex m_Mutex{};
    std::vector<std::thread> m_Workers{};
    std::condition_variable m_ConditionVariable{};
    EditorAssetManager* m_AssetManager{ nullptr };
};

class AssetSerializer;

class EditorAssetManager final : public Fussion::AssetManagerBase {
    // NOTE: Is there a better way/abstraction to expose internals to the worker pool?
    friend WorkerPool;
public:
    using Registry = std::unordered_map<Fsn::AssetHandle, EditorAssetMetadata>;

    EditorAssetManager();
    virtual ~EditorAssetManager() override;

    virtual auto GetAsset(Fsn::AssetHandle handle, Fsn::AssetType type) -> Fussion::Asset* override;
    virtual auto GetAsset(std::string const& path, Fussion::AssetType type) -> Fussion::Asset* override;

    virtual bool IsAssetLoaded(Fsn::AssetHandle handle) override;
    virtual bool IsAssetHandleValid(Fsn::AssetHandle handle) const override;
    virtual bool IsAssetVirtual(Fussion::AssetHandle handle) override;
    virtual auto CreateVirtualAsset(Ref<Fussion::Asset> const& asset, std::string_view name, std::filesystem::path const& path) -> Fussion::AssetHandle override;
    virtual auto GetAssetMetadata(Fussion::AssetHandle handle) -> Fussion::AssetMetadata* override;

    bool IsAssetLoading(Fussion::AssetHandle handle);

    bool IsPathAnAsset(std::filesystem::path const& path, bool include_virtual = false) const;
    auto GetMetadata(std::filesystem::path const& path) const -> Maybe<EditorAssetMetadata>;
    auto GetMetadata(Fsn::AssetHandle handle) const -> EditorAssetMetadata;
    void RegisterAsset(std::filesystem::path const& path, Fussion::AssetType type);

    auto GetRegistry() -> Fussion::ThreadProtected<Registry>& { return m_Registry; }

    template<std::derived_from<Fsn::Asset> T>
    auto CreateAsset(std::filesystem::path const& path) -> Fsn::AssetRef<T>
    {
        auto normal_path = path.lexically_normal();
        Fussion::AssetHandle handle;

        m_Registry.Access([&](auto& registry) {
            registry[handle] = EditorAssetMetadata{
                .Type = T::GetStaticType(),
                .Path = normal_path,
                .Name = path.filename().string(),
                .IsVirtual = false,
                .DontSerialize = false,
                .Handle = handle,
            };
        });

        m_LoadedAssets[handle] = MakeRef<T>();

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

        SaveAsset(handle);

        Serialize();

        return Fsn::AssetRef<T>(handle);
    }

    void SaveAsset(Fussion::AssetHandle handle);
    void SaveAsset(Ref<Fussion::Asset> const& asset);

    void Serialize();
    void Deserialize();
    void RefreshAsset(Fussion::AssetHandle handle);

    // NOTE: I'm not sure if this is a good way to go about it. Could some kind of callback
    // be used instead?
    void CheckForLoadedAssets();

private:
    void LoadAsset(Fussion::AssetHandle handle, Fussion::AssetType type);

    Fussion::ThreadProtected<Registry> m_Registry{};
    std::unordered_map<Fsn::AssetHandle, Ref<Fsn::Asset>> m_LoadedAssets;

    std::map<Fsn::AssetType, Ptr<AssetSerializer>> m_AssetSerializers{};

    Ptr<Fussion::FileWatcher> m_EditorWatcher{};

    WorkerPool m_WorkerPool{ this };
};
