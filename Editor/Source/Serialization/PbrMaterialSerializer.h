#pragma once
#include "AssetSerializer.h"

class PbrMaterialSerializer final : public AssetSerializer {
public:
    Ref<Fussion::Asset> Load(AssetMetadata metadata) override;
    void Save(AssetMetadata metadata, Ref<Fussion::Asset> const& asset) override;
};
