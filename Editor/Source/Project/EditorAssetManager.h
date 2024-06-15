#pragma once
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Assets/AssetManagerBase.h"
#include "Fussion/Assets/Asset.h"
#include "Fussion/Core/Types.h"

#include <filesystem>
#include <unordered_map>
#include <concepts>

struct AssetMetadata
{
    Fsn::AssetType Type = Fsn::AssetType::Invalid;
    std::filesystem::path Path;

    bool IsVirtual = false;
    bool DontSerialize = false;

    bool IsValid() const { return Type != Fsn::AssetType::Invalid; }
};

class AssetSerializer;
class EditorAssetManager final: public Fussion::AssetManagerBase
{
public:
    EditorAssetManager();

    Fussion::Asset* GetAsset(Fsn::AssetHandle handle, Fsn::AssetType type) override;
    bool IsAssetLoaded(Fsn::AssetHandle handle) override;
    bool IsAssetHandleValid(Fsn::AssetHandle handle) override;

    bool IsPathAnAsset(std::filesystem::path const& path) const;
    AssetMetadata GetMetadata(std::filesystem::path const& path) const;

    template<std::derived_from<Fsn::Asset> T>
    Fsn::AssetRef<T> CreateAsset(std::filesystem::path const& path)
    {
        Fussion::AssetHandle handle;
        m_Registry[handle] = AssetMetadata{
            .Type = T::GetStaticType(),
            .Path = path,
            .IsVirtual = false,
            .DontSerialize = false,
        };

        m_LoadedAssets[handle] = MakeRef<T>();

        // Create the necessary directories, recursively.
        auto base_path = path;
        base_path.remove_filename();
        if (!base_path.empty()) {
            try {
                std::filesystem::create_directories(base_path);
            }
            catch (std::exception& e) {
                LOG_DEBUGF("Exception caught in create_directories: '{}'\npath {}", e.what(), path.string());
            }
        }

        SaveAsset(handle);

        Serialize();

        return Fsn::AssetRef<T>(handle, this);
    }

    void SaveAsset(Fussion::AssetHandle handle);

    void Serialize();
    void Deserialize();

private:
    std::unordered_map<Fsn::AssetHandle, AssetMetadata> m_Registry{};
    std::unordered_map<Fsn::AssetHandle, Ref<Fsn::Asset>> m_LoadedAssets{};

    std::map<Fsn::AssetType, Ptr<AssetSerializer>> m_AssetSerializers{};
};
