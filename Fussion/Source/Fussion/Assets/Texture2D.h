#pragma once
#include "Asset.h"
#include "Fussion/Renderer/Image.h"
#include "Generated/Texture2D_reflect_generated.h"

namespace Fussion
{
    struct Texture2DSpec
    {
        s32 Width, Height;

        f32 Aspect() const { return cast(f32, Width) / cast(f32, Height); }
    };

    REFLECT_CLASS()
    class Texture2D: public Asset
    {
        REFLECT_GENERATED_BODY()
    public:
        static Ref<Texture2D> Create(u8* data, Texture2DSpec spec);

        Ref<Image>& GetImage() { return m_Image; }

        Texture2DSpec Spec() const { return m_Spec; }
    private:
        Ref<Image> m_Image{};
        Texture2DSpec m_Spec{};
    };
}
