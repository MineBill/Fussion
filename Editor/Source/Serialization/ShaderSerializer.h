#pragma once
#include "AssetSerializer.h"

class ShaderSerializer final : public AssetSerializer {
public:
    Ref<Fussion::Asset> Load(AssetMetadata metadata) override;
};
