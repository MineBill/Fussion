#include "e5pch.h"
#include "Entity.h"
#include "Fussion/Scene/Scene.h"

#include <algorithm>

namespace Fussion
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

    // void Entity::AddComponent(Reflect::TypeId const& type_id)
    // {
    //     auto type_info = Reflect::TypeInfoRegistry::GetTypeInfo(type_id);
    //     if (!type_info.GetType().IsValid())
    //         return;
    //
    //     if (HasComponent(type_id)) {
    //         LOG_WARNF("Attempted to add an already existing component of type {}", type_info.GetType().GetPrettyTypeName());
    //         return;
    //     }
    //
    //     auto const id = type_id.GetHash();
    //     // type_info.GetFunctionInfo("ctor");
    //     auto component = MakeRef<Component>();
    //     component.reset(cast(Component*, type_info.Construct()));
    //     m_Components[id] = component;
    // }
    //
    // bool Entity::HasComponent(Reflect::TypeId const& type_id) const
    // {
    //     auto const id = type_id.GetHash();
    //     return m_Components.contains(id);
    // }

    void Entity::OnUpdate(f32 const delta)
    {
        for (auto& [id, component]: m_Components) {
            component->OnUpdate(delta);
        }
    }
}