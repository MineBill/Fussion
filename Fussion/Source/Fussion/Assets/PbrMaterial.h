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
    AssetType GetType() const override { return GetStaticType(); }

    Color ObjectColor{};
    f32 Metallic{};
    f32 Roughness{};

    AssetRef<Texture2D> AlbedoMap{};

    struct MaterialBlock {
        Color ObjectColor;
        f32 Metallic;
        f32 Roughness;
    };

    RHI::UniformBuffer<MaterialBlock> MaterialUniformBuffer;
};
}
