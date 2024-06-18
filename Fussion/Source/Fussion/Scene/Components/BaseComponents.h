#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Core/UUID.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Log/Log.h"

namespace Fussion
{
    class PointLight: public Component
    {
        META_HPP_ENABLE_POLY_INFO(Component)
    public:
        PointLight() = default;
        COMPONENT(PointLight)
    };
}

namespace Fsn = Fussion;