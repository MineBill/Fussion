#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Core/UUID.h"
#include "Reflect/Reflect.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Log/Log.h"
#include "Generated/BaseComponents_reflect_generated.h"

namespace Fussion
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

namespace Fsn = Fussion;