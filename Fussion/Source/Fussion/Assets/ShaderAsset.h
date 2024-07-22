#pragma once
#include "Fussion/Assets/Asset.h"
#include "Fussion/RHI/Shader.h"

namespace Fussion {
class ShaderAsset : public Asset {
public:
    static Ref<ShaderAsset> Create(std::span<u8> data);

    static AssetType GetStaticType() { return AssetType::Shader; }
    AssetType GetType() const override { return GetStaticType(); }

    Ref<RHI::Shader> GetShader() const { return m_Shader; }

private:
    Ref<RHI::Shader> m_Shader;
};
}
