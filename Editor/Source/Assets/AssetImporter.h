#pragma once
#include "Reflect/Reflect.h"
#include "Generated/AssetImporter_reflect_generated.h"

/**
 * Editor-Only. Raw file -> Asset.
 */
REFLECT_CLASS()
class AssetImporter: REFLECT_BASE
{
    REFLECT_GENERATED_BODY()
public:
};

#define ASSET_IMPORTER()

class TextureImporter: public AssetImporter
{
public:
    static std::vector<const char*> SupportedFiletypes()
    {
        return {"png", "jpg", "jpeg"};
    }
};