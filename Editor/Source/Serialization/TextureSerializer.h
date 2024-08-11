#pragma once
#include "AssetSerializer.h"

class TextureSerializer final : public AssetSerializer
{
public:
    void Save(EditorAssetMetadata metadata, Ref<Fussion::Asset> const& asset) override;
    Ref<Fussion::Asset> Load(EditorAssetMetadata metadata) override;
};
