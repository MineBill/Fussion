#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Core/UUID.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Log/Log.h"

namespace Fussion
{
    class PointLight: public Component
    {
    public:
        PointLight() = default;
        COMPONENT(PointLight)
    };
}

namespace Fsn = Fussion;