#pragma once
#include "AssetRef.h"
#include "Texture2D.h"
#include "Fussion/Assets/Asset.h"
#include "Fussion/Math/Color.h"
#include "Fussion/RHI/UniformBuffer.h"

namespace Fussion {
class PbrMaterial final : public Asset {
public:
    PbrMaterial();

    static AssetType GetStaticType() { return AssetType::PbrMaterial; }
    virtual AssetType GetType() const override { return GetStaticType(); }

    Color ObjectColor{};
    f32 Metallic{};
    f32 Roughness{};

    AssetRef<Texture2D> AlbedoMap{};
    AssetRef<Texture2D> NormalMap{};
    AssetRef<Texture2D> AmbientOcclusionMap{};
    AssetRef<Texture2D> MetallicRoughnessMap{};
    AssetRef<Texture2D> EmissiveMap{};

    struct MaterialBlock {
        Color ObjectColor;
        f32 Metallic;
        f32 Roughness;
    };

    RHI::UniformBuffer<MaterialBlock> MaterialUniformBuffer;
};
}
