#pragma once
#include "Engin5/Core/Core.h"
#include "Engin5/Core/UUID.h"
#include "Reflect/Reflect.h"
#include "Generated/Asset_reflect_generated.h"

namespace Engin5
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

        friend class AssetManager;
    };
}
