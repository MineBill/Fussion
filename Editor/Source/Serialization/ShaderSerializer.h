#pragma once
#include "AssetSerializer.h"

class ShaderSerializer final : public AssetSerializer {
public:
    virtual Ref<Fussion::Asset> load(EditorAssetMetadata metadata) override;
};
