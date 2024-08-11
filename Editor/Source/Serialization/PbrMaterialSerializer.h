#pragma once
#include "AssetSerializer.h"

class PbrMaterialSerializer final : public AssetSerializer {
public:
    Ref<Fussion::Asset> Load(EditorAssetMetadata metadata) override;
    void Save(EditorAssetMetadata metadata, Ref<Fussion::Asset> const& asset) override;
};
