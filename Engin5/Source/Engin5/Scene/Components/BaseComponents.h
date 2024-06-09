#pragma once
#include "Engin5/Core/Types.h"
#include "Engin5/Core/UUID.h"
#include "Reflect/Reflect.h"
#include "Engin5/Scene/Component.h"
#include "Engin5/Log/Log.h"
#include "Generated/BaseComponents_reflect_generated.h"

namespace Engin5
{
    REFLECT_CLASS(Meta(Category("Rendering/Lights")))
    class PointLight: public Component
    {
        REFLECT_GENERATED_BODY();
    public:
        PointLight() = default;
        COMPONENT(PointLight)
    };
}