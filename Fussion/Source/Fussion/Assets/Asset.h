#pragma once
#include "Fussion/Core/Core.h"
#include "Fussion/Core/UUID.h"
#include "Reflect/Reflect.h"
#include "Generated/Asset_reflect_generated.h"

namespace Fussion
{
    enum class AssetType
    {
        Invalid,

        Image,
        Script,
        Mesh,
        PbrMaterial,
        Scene,
        Shader,
        Texture,
        Texture2D,
        HDRTexture,
    };

    using AssetHandle = UUID;

    REFLECT_CLASS()
    class Asset: REFLECT_BASE
    {
        REFLECT_GENERATED_BODY()
    public:
        require_results AssetType GetType() const { return m_Type; }
        require_results AssetHandle GetHandle() const { return m_Handle; }

    private:
        AssetType m_Type{AssetType::Invalid};
        AssetHandle m_Handle{0};

        friend class AssetManagerBase;
    };
}
