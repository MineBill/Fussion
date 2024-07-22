#pragma once
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

    struct MaterialBlock {
        Color ObjectColor;
    };

    RHI::UniformBuffer<MaterialBlock> MaterialUniformBuffer;
};
}
