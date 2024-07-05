#pragma once
#include "Fussion/Assets/Asset.h"

/**
 * Editor-Only. Raw file -> Asset.
 */
class AssetImporter
{
public:
    virtual ~AssetImporter() = default;
    virtual Fussion::Asset* Load(std::filesystem::path path) = 0;
    virtual std::vector<const char*> GetSupportedExt() = 0;
};