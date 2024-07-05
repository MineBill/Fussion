#include "BaseComponents.h"

#include "Scene/Entity.h"

namespace Fussion {
void MoverComponent::OnUpdate(f32 delta)
{
    m_Owner->Transform.Position.X += delta * Speed;
}
}
