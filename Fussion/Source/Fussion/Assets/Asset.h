#pragma once
#include "Fussion/Core/Core.h"
#include "Fussion/Core/UUID.h"
#include "Fussion/ReflRegistrar.h"

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

class Asset: public std::enable_shared_from_this<Asset> {
    friend ReflRegistrar;
public:
    virtual ~Asset() = default;

    [[nodiscard]] AssetType GetType() const { return m_Type; }
    [[nodiscard]] AssetHandle GetHandle() const { return m_Handle; }

    template<typename T>
    Ref<T> As()
    {
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }

private:
    AssetType m_Type{ AssetType::Invalid };
    AssetHandle m_Handle{ 0 };

    friend class AssetManagerBase;
};
}
