#include "BaseComponents.h"

#include "Debug/Debug.h"
#include "Scene/Entity.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    void PointLight::OnUpdate(f32) {}

    void PointLight::OnDraw(RenderContext& context)
    {
        if (!context.RenderFlags.Test(RenderState::LightCollection))
            return;
        auto light = GPUPointLight{
            .Position = m_Owner->Transform.Position,
            .Color = Color::White,
            .Radius = Radius,
        };
        context.PointLights.push_back(light);
    }

    void PointLight::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(Offset);
        FSN_SERIALIZE_MEMBER(Radius);
    }

    void PointLight::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(Offset);
        FSN_DESERIALIZE_MEMBER(Radius);
    }

    void DebugDrawer::OnDebugDraw(DebugDrawContext& ctx)
    {
        (void)ctx;

        if (DrawType == Type::Box) {
            Debug::DrawCube(m_Owner->Transform.Position, m_Owner->Transform.EulerAngles, Vector3::One * Size);
        } else if (DrawType == Type::Sphere) {
            Debug::DrawSphere(m_Owner->Transform.Position, m_Owner->Transform.EulerAngles, Size);
        }
    }

    void DebugDrawer::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(Size);
        FSN_SERIALIZE_MEMBER(DrawType);
    }

    void DebugDrawer::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(Size);
        FSN_DESERIALIZE_MEMBER(DrawType);
    }

    void MoverComponent::OnUpdate(f32 delta)
    {
        m_Owner->Transform.Position.X += delta * Speed;
    }

    void MoverComponent::Serialize(Serializer& ctx) const
    {
        Component::Serialize(ctx);
        FSN_SERIALIZE_MEMBER(Speed);
    }

    void MoverComponent::Deserialize(Deserializer& ctx)
    {
        Component::Deserialize(ctx);
        FSN_DESERIALIZE_MEMBER(Speed);
    }
}
