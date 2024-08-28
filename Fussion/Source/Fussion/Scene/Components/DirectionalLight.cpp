#include "FussionPCH.h"
#include "DirectionalLight.h"

#include "Debug/Debug.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void DirectionalLight::OnEnabled()
    {
        Component::OnEnabled();
    }

    void DirectionalLight::OnDisabled()
    {
        Component::OnDisabled();
    }

    void DirectionalLight::OnUpdate([[maybe_unused]] f32 delta) {}

    void DirectionalLight::OnDebugDraw(DebugDrawContext& ctx)
    {
        (void)ctx;

        auto start = m_Owner->Transform.Position;
        auto end = start + m_Owner->Transform.GetForward();
        Debug::DrawLine(start, end, 0.0f, Color::Green);
        Debug::DrawCube(end, m_Owner->Transform.EulerAngles, Vector3::One * 0.1f);
    }

    void DirectionalLight::OnDraw(RenderContext& context)
    {
        if (!context.RenderFlags.Test(RenderState::LightCollection))
            return;

        context.DirectionalLights.push_back(GPUDirectionalLight{
            {
                .Direction = Vector4{ -m_Owner->Transform.GetForward() },
                .Color = LightColor,
            },
            SplitLambda,
        });
    }

    void DirectionalLight::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(LightColor);
        FSN_SERIALIZE_MEMBER(SplitLambda);
    }

    void DirectionalLight::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(LightColor);
        FSN_DESERIALIZE_MEMBER(SplitLambda);
    }
}
