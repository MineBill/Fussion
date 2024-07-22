#include "W:\source\projects\Fussion\build\.gens\Fussion\windows\x64\debug\Fussion\Source\e5pch.h"
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

void DirectionalLight::OnUpdate(f32 delta)
{
    auto rotation = glm::mat3(glm::eulerAngleZXY(
        glm::radians(m_Owner->Transform.EulerAngles.Z),
        glm::radians(m_Owner->Transform.EulerAngles.X),
        glm::radians(m_Owner->Transform.EulerAngles.Y)));

    auto direction = rotation * Vector3::Forward;

    Debug::DrawLine(m_Owner->Transform.Position, m_Owner->Transform.Position + direction, 0.0f, Color::Green);
}

void DirectionalLight::OnDraw(RHI::RenderContext& context)
{
    if (!context.RenderFlags.Test(RHI::RenderState::LightCollection))
        return;

    auto rotation = glm::mat3(glm::eulerAngleZXY(
        glm::radians(m_Owner->Transform.EulerAngles.Z),
        glm::radians(m_Owner->Transform.EulerAngles.X),
        glm::radians(m_Owner->Transform.EulerAngles.Y)));

    auto direction = rotation * Vector3::Forward;

    context.DirectionalLights.push_back(RHI::DirectionalLightData{
        .Direction = Vector4{ direction },
        .Color = Color::White,
    });
}
}
