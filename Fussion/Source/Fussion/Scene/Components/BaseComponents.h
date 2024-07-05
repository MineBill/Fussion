#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Core/UUID.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Log/Log.h"

#define FSN_FIELD(...)

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

    FSN_FIELD()
    f32 Speed;
};
}

namespace Fsn = Fussion;
