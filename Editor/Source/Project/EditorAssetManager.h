#pragma once
#include "Engin5/Assets/AssetManager.h"
#include "Engin5/Assets/Asset.h"
#include "Engin5/Core/Types.h"

#include <filesystem>
#include <unordered_map>
#include <set>

struct AssetMetadata
{
    Engin5::AssetType Type = Engin5::AssetType::Invalid;
    std::filesystem::path Path;

    bool IsVirtual = false;
    bool DontSerialize = false;
};

class EditorAssetManager final: public Engin5::AssetManager
{
public:
    Ref<Engin5::Asset> GetAsset(Engin5::AssetHandle handle) override;
    bool IsAssetLoaded(Engin5::AssetHandle handle) override;
    bool IsAssetHandleValid(Engin5::AssetHandle handle) override;

private:
    std::unordered_map<Engin5::AssetHandle, AssetMetadata> m_Registry{};
    std::unordered_map<Engin5::AssetHandle, Ref<Engin5::Asset>> m_LoadedAssets{};
};
