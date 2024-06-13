#pragma once
#include "Reflect/Reflect.h"
#include "Engin5/Assets/Asset.h"
#include "Generated/AssetImporter_reflect_generated.h"
REFLECT_CPP_INCLUDE("Engin5/Assets/Asset.h")

/**
 * Editor-Only. Raw file -> Asset.
 */
REFLECT_CLASS(Abstract)
class AssetImporter: REFLECT_BASE
{
    REFLECT_GENERATED_BODY()
public:

    virtual Engin5::Asset* Load(std::filesystem::path path) = 0;
    virtual std::vector<const char*> GetSupportedExt() = 0;
};