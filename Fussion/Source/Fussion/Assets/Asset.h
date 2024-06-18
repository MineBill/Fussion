#pragma once
#include "Fussion/Core/Core.h"
#include "Fussion/Core/UUID.h"

namespace Fussion {
enum class AssetType {
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

class Asset {
public:
    [[nodiscard]] AssetType GetType() const { return m_Type; }
    [[nodiscard]] AssetHandle GetHandle() const { return m_Handle; }

private:
    AssetType m_Type{ AssetType::Invalid };
    AssetHandle m_Handle{ 0 };

    friend class AssetManagerBase;
};
}
