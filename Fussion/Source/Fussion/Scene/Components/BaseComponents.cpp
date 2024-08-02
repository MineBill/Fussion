#include "BaseComponents.h"

#include "Debug/Debug.h"
#include "Scene/Entity.h"

namespace Fussion {
    void PointLight::OnUpdate(f32) {}

    void PointLight::OnDraw(RHI::RenderContext& context)
    {
        if (!context.RenderFlags.Test(RHI::RenderState::LightCollection))
            return;
        auto light = RHI::PointLightData{
            .Position = m_Owner->Transform.Position,
            .Color = Color::White,
            .Radius = Radius,
        };
        context.PointLights.push_back(light);
    }

    void DebugDrawer::OnDebugDraw()
    {
        if (DrawType == Type::Box) {
            Debug::DrawCube(m_Owner->Transform.Position, m_Owner->Transform.EulerAngles, Vector3::One * Size);
        } else if (DrawType == Type::Sphere) {
            Debug::DrawSphere(m_Owner->Transform.Position, m_Owner->Transform.EulerAngles, Size);
        }
    }

    void MoverComponent::OnUpdate(f32 delta)
    {
        m_Owner->Transform.Position.X += delta * Speed;
    }
}
