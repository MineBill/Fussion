#pragma once
#include "Scene/Component.h"

namespace Fussion {
class DirectionalLight final : public Component {
public:
    COMPONENT(DirectionalLight)

    void OnEnabled() override;
    void OnDisabled() override;
    void OnUpdate(f32 delta) override;

    void OnDraw(RHI::RenderContext& context) override;
};

}
