#pragma once
#include "Assets/AssetImporter.h"

class MeshImporter final : public AssetImporter {
public:
    Fussion::Asset* Load(std::filesystem::path path) override;

    std::vector<const char*> GetSupportedExt() override
    {
        return { ".glb", ".gltf" };
    }
};
