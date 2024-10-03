#pragma once
#include "AssetSerializer.h"

class TextureSerializer final : public AssetSerializer {
public:
    virtual auto Load(EditorAssetMetadata metadata) -> Ref<Fussion::Asset> override;
};
