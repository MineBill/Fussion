#pragma once
#include "AssetSerializer.h"

class SceneSerializer: public AssetSerializer
{
public:

    void Save(AssetMetadata metadata, Engin5::Asset* asset) override;
    Engin5::Asset* Load(AssetMetadata metadata) override;
};
