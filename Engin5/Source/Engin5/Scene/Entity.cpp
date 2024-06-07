#include "e5pch.h"
#include "Entity.h"
#include "Engin5/Scene/Scene.h"

#include <algorithm>

namespace Engin5
{
    void Entity::SetParent(Entity const& parent)
    {
        m_Scene->GetEntity(m_Parent)->RemoveChild(*this);
        m_Scene->GetEntity(parent)->AddChild(*this);
        m_Parent = parent.m_Handle;
    }

    void Entity::AddChild(Entity const& child)
    {
        m_Children.push_back(child.m_Handle);
    }

    void Entity::RemoveChild(Entity const& child)
    {
        if (auto const pos = std::ranges::find(m_Children, child.m_Handle); pos != std::end(m_Children)) {
            std::erase(m_Children, *pos);
        }
    }

    void Entity::AddComponent(Reflect::TypeId const& type_id)
    {
        auto id = type_id.GetHash();
        auto type_info = Reflect::TypeInfoRegistry::GetTypeInfo(type_id);
        // type_info.GetFunctionInfo("ctor");
    }
}