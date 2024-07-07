#pragma once
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Assets/AssetManagerBase.h"
#include "Fussion/Assets/Asset.h"
#include "Fussion/Core/Types.h"

#include <filesystem>
#include <unordered_map>
#include <concepts>

struct AssetMetadata {
    Fsn::AssetType Type = Fsn::AssetType::Invalid;
    std::filesystem::path Path;

    bool IsVirtual = false;
    bool DontSerialize = false;

    Fussion::AssetHandle Handle;
    bool IsValid() const { return Type != Fsn::AssetType::Invalid; }
};

class AssetSerializer;

class EditorAssetManager final : public Fussion::AssetManagerBase {
public:
    using Registry = std::unordered_map<Fsn::AssetHandle, AssetMetadata>;

    EditorAssetManager();

    auto GetAsset(Fsn::AssetHandle handle, Fsn::AssetType type) -> Fussion::Asset* override;
    bool IsAssetLoaded(Fsn::AssetHandle handle) const override;
    bool IsAssetHandleValid(Fsn::AssetHandle handle) const override;

    bool IsPathAnAsset(std::filesystem::path const& path) const;
    auto GetMetadata(std::filesystem::path const& path) const -> AssetMetadata;
    auto GetMetadata(Fsn::AssetHandle handle) const -> AssetMetadata;
    void RegisterAsset(std::filesystem::path const& path, Fussion::AssetType type);

    auto GetRegistry() const -> Registry const& { return m_Registry; }

    template<std::derived_from<Fsn::Asset> T>
    auto CreateAsset(std::filesystem::path const& path) -> Fsn::AssetRef<T>
    {
        auto normal_path = path.lexically_normal();
        Fussion::AssetHandle handle;
        m_Registry[handle] = AssetMetadata{
            .Type = T::GetStaticType(),
            .Path = normal_path,
            .IsVirtual = false,
            .DontSerialize = false,
            .Handle = handle,
        };

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

    void Serialize();
    void Deserialize();

private:
    Registry m_Registry{};
    std::unordered_map<Fsn::AssetHandle, Ref<Fsn::Asset>> m_LoadedAssets{};

    std::map<Fsn::AssetType, Ptr<AssetSerializer>> m_AssetSerializers{};
};
