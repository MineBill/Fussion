#pragma once
#include "Scene/Component.h"

namespace Fussion {
    class Camera: public Component
    {
    public:
        f32 FieldOfView = 10.22;
    private:
        f32 m_HiddenValue = 69.0;
    };
}
