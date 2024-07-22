#pragma once
#include "Asset.h"
#include "Fussion/RHI/Image.h"

namespace Fussion {
struct Texture2DSpec {
    s32 Width, Height;

    f32 Aspect() const { return CAST(f32, Width) / CAST(f32, Height); }
};

class Texture2D : public Asset {
public:
    static Ref<Texture2D> Create(u8* data, Texture2DSpec spec);

    Ref<RHI::Image>& GetImage() { return m_Image; }

    Texture2DSpec Spec() const { return m_Spec; }

    static AssetType GetStaticType() { return AssetType::Texture2D; }
    AssetType GetType() const override { return GetStaticType(); }

private:
    Ref<RHI::Image> m_Image{};
    Texture2DSpec m_Spec{};
};
}
