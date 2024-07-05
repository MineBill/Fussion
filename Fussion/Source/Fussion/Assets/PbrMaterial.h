#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/Math/Color.h"

namespace Fussion {
class PbrMaterial final : public Asset {
public:
    static AssetType GetStaticType() { return AssetType::PbrMaterial; }

    Color Color;

private:

};
}
