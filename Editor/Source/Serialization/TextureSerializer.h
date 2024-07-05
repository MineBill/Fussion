#pragma once
#include "AssetSerializer.h"

class TextureSerializer final : public AssetSerializer
{
public:
    void Save(AssetMetadata metadata, Ref<Fussion::Asset> const& asset) override;
    Ref<Fussion::Asset> Load(AssetMetadata metadata) override;
};
