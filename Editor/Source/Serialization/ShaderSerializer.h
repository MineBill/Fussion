#pragma once
#include "AssetSerializer.h"

class ShaderSerializer final : public AssetSerializer {
public:
    virtual Ref<Fussion::Asset> Load(EditorAssetMetadata metadata) override;
};
