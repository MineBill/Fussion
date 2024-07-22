#pragma once
#include "Fussion/Scene/Component.h"

namespace Fussion {
    class Camera: public Component
    {
    public:
        Camera();
        ~Camera() override;
        COMPONENT(Camera)

        void OnCreate() override;

        f32 FieldOfView{10.22f};
        s32 SignedType = 0;
        u32 UnsignedType = 0;
        std::string AStringToo{"Pepegas"};
    private:
        f32 m_HiddenValue = 69.0;

    };
}
