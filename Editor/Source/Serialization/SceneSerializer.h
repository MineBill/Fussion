#pragma once
#include "AssetSerializer.h"

class SceneSerializer final : public AssetSerializer
{
public:
    virtual auto Save(EditorAssetMetadata metadata, Ref<Fussion::Asset> const& asset) -> void override;
    virtual auto Load(EditorAssetMetadata metadata) -> Ref<Fussion::Asset> override;
};
