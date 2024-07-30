#pragma once
#include "Scene/Component.h"

namespace Fussion {
class DirectionalLight final : public Component {
public:
    COMPONENT(DirectionalLight)

    virtual void OnEnabled() override;
    virtual void OnDisabled() override;
    virtual void OnUpdate(f32 delta) override;

#if USE_DEBUG_DRAW
    virtual void OnDebugDraw() override;
#endif


    virtual void OnDraw(RHI::RenderContext& context) override;
};

}
