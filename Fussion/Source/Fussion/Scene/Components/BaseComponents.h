#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Core/UUID.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Log/Log.h"

namespace Fussion {
class PointLight : public Component {
    META_HPP_ENABLE_POLY_INFO(Component)
public:
    COMPONENT_DEFAULT(PointLight)
};

class MoverComponent : public Component {
    META_HPP_ENABLE_POLY_INFO(Component)

public:
    COMPONENT_DEFAULT(MoverComponent)

    void OnUpdate(f32 delta) override;

    f32 Speed{0.1f};
};
}

namespace Fsn = Fussion;
