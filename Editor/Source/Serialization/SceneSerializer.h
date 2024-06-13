#pragma once
#include "AssetSerializer.h"

class SceneSerializer: public AssetSerializer
{
public:

    void Save(AssetMetadata metadata, Fussion::Asset* asset) override;
    Fussion::Asset* Load(AssetMetadata metadata) override;
};
