#pragma once
#include "Fussion/Core/Types.h"
#include "Fussion/Core/Uuid.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Log/Log.h"

namespace Fussion {
class PointLight : public Component {
public:
    COMPONENT_DEFAULT(PointLight)

    void OnUpdate(f32 delta) override;
    void OnDraw(RHI::RenderContext& context) override;

    f32 Radius{ 10.0f };
    Vector3 Offset{};
};

class DebugDrawer : public Component {
public:
    enum class Type {
        Sphere,
        Box,
    };

    COMPONENT_DEFAULT(DebugDrawer)

    void OnUpdate(f32 delta) override;

    Type DrawType{};

    f32 Size{ 0.0f };
};

class MoverComponent : public Component {
public:
    COMPONENT_DEFAULT(MoverComponent)

    void OnUpdate(f32 delta) override;

    f32 Speed{ 0.1f };
};
}

namespace Fsn = Fussion;
