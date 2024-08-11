#pragma once
#include "AssetSerializer.h"

class MeshSerializer : public AssetSerializer {
public:
    void Save(EditorAssetMetadata metadata, Ref<Fussion::Asset> const& asset) override;
    Ref<Fussion::Asset> Load(EditorAssetMetadata metadata) override;
};
