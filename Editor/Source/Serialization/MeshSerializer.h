#pragma once
#include "AssetSerializer.h"

class MeshSerializer final : public AssetSerializer {
public:
    virtual auto save(EditorAssetMetadata metadata, Ref<Fussion::Asset> const& asset) -> void override;
    virtual auto load(EditorAssetMetadata metadata) -> Ref<Fussion::Asset> override;
};
