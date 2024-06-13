#pragma once
#include "Engin5/Assets/AssetManagerBase.h"
#include "Engin5/Assets/Asset.h"
#include "Engin5/Core/Types.h"

#include <filesystem>
#include <unordered_map>
#include <concepts>

#include "Assets/AssetImporter.h"

struct AssetMetadata
{
    Engin5::AssetType Type = Engin5::AssetType::Invalid;
    std::filesystem::path Path;

    bool IsVirtual = false;
    bool DontSerialize = false;
};

class AssetSerializer;
class EditorAssetManager final: public Engin5::AssetManagerBase
{
public:
    EditorAssetManager();

    Engin5::Asset* GetAsset(Engin5::AssetHandle handle, Engin5::AssetType type) override;
    bool IsAssetLoaded(Engin5::AssetHandle handle) override;
    bool IsAssetHandleValid(Engin5::AssetHandle handle) override;

    template<std::derived_from<Engin5::Asset> T>
    Engin5::AssetHandle CreateAsset(std::filesystem::path const& path)
    {
        Engin5::AssetHandle handle;
        m_Registry[handle] = AssetMetadata{
            .Type = T::GetStaticType(),
            .Path = path,
            .IsVirtual = false,
            .DontSerialize = false,
        };

        m_LoadedAssets[handle] = MakeRef<T>();
        SaveAsset(handle);

        Serialize();

        return handle;
    }

    void SaveAsset(Engin5::AssetHandle handle);

    void Serialize();
    void Deserialize();

private:
    std::unordered_map<Engin5::AssetHandle, AssetMetadata> m_Registry{};
    std::unordered_map<Engin5::AssetHandle, Ref<Engin5::Asset>> m_LoadedAssets{};

    std::map<Engin5::AssetType, Ptr<AssetSerializer>> m_AssetSerializers{};
};
