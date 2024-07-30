#include "e5pch.h"
#include "DirectionalLight.h"

#include "Debug/Debug.h"
#include "Scene/Entity.h"

#include <glm/gtx/euler_angles.hpp>

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

void DirectionalLight::OnDebugDraw()
{
    auto start = m_Owner->Transform.Position;
    auto end = start + m_Owner->Transform.GetForward();
    Debug::DrawLine(start, end, 0.0f, Color::Green);
    Debug::DrawCube(end, m_Owner->Transform.EulerAngles, Vector3::One * 0.1f);
}

void DirectionalLight::OnDraw(RHI::RenderContext& context)
{
    if (!context.RenderFlags.Test(RHI::RenderState::LightCollection))
        return;

    context.DirectionalLights.push_back(RHI::DirectionalLightData{
        .Direction = Vector4{ m_Owner->Transform.GetForward() },
        .Color = Color::White,
    });
}
}
