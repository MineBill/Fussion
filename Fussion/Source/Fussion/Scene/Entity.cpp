#include "e5pch.h"
#include "Entity.h"
#include "Fussion/Scene/Scene.h"

#include "Components/Camera.h"

namespace Fussion
{
    void Entity::SetParent(Entity const& new_parent)
    {
        if (IsGrandchild(new_parent.m_Handle))
            return;

        if (auto p = m_Scene->GetEntity(m_Parent)) {
            p->RemoveChild(*this);
        }
        m_Scene->GetEntity(new_parent)->AddChild(*this);
        m_Parent = new_parent.m_Handle;
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

    auto Entity::AddComponent(meta_hpp::class_type type) -> Ref<Component>
    {
        VERIFY(type.is_derived_from(meta_hpp::resolve_type<Component>()),
            "Attempted to add a component that doesn't derive from Component, weird.");

        auto component = MakeRef<Component>();

        auto data = type.create(this);

        auto ptr = *CAST(Component**, data.get_data());
        Ref<Component> comp;
        comp.reset(ptr);

        comp->OnCreate();
        m_Components[type.get_hash()] = comp;
        return comp;
    }

    auto Entity::HasComponent(meta_hpp::class_type type) const -> bool
    {
        return m_Components.contains(type.get_hash());
    }

    void Entity::OnUpdate(f32 const delta)
    {
        for (auto& [id, component]: m_Components) {
            component->OnUpdate(delta);
        }
    }

    bool Entity::IsGrandchild(UUID handle) const
    {
        if (m_Children.empty()) {
            return false;
        }

        for (auto const& child : m_Children) {
            if (child == handle) {
                return true;
            }

            if (auto en = m_Scene->GetEntity(child); en->IsGrandchild(handle)) {
                return true;
            }
        }
        return false;
    }
}
