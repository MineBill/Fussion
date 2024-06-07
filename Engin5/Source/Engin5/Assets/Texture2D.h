#pragma once
#include "Asset.h"
#include "Engin5/Renderer/Image.h"
#include "Generated/Texture2D_reflect_generated.h"

namespace Engin5
{
    struct Texture2DSpec
    {
        s32 Width, Height;
    };

    REFLECT_CLASS()
    class Texture2D: public Asset
    {
        REFLECT_GENERATED_BODY()
    public:
        static Ref<Texture2D> Create(u8* data, Texture2DSpec spec);

        Ref<Image>& GetImage() { return m_Image; }

    private:
        Ref<Image> m_Image{};
        Texture2DSpec m_Spec{};
    };
}
