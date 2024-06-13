#pragma once
#include "Scene/Component.h"
#include "Reflect/Reflect.h"
#include "Generated/Camera_reflect_generated.h"

namespace Engin5 {
    REFLECT_CLASS()
    class Camera: public Component
    {
        REFLECT_GENERATED_BODY()
    public:
        REFLECT_PROPERTY(ShowInEditor, Meta(Range(10.0|200.0)))
        f32 FieldOfView = 10.22;
    private:
        REFLECT_PROPERTY(ShowInEditor)
        f32 m_HiddenValue = 69.0;
    };
}
