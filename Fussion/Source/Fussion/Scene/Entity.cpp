#include "e5pch.h"
#include "Entity.h"
#include "Fussion/Scene/Scene.h"

#include "Components/Camera.h"

#include <ranges>
#include <tracy/Tracy.hpp>

namespace Fussion {
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

void Entity::SetEnabled(bool enabled)
{
    m_Enabled = enabled;
    if (enabled) {
        for (auto& [id, component] : m_Components) {
            (void)id;
            component->OnEnabled();
        }
    } else {
        for (auto& [id, component] : m_Components) {
            (void)id;
            component->OnDisabled();
        }
    }
}

bool Entity::IsEnabled() const
{
    if (!m_Enabled)
        return false;
    if (auto parent = m_Scene->GetEntity(m_Parent)) {
        return parent->IsEnabled();
    }
    return false;
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

auto Entity::GetComponent(meta_hpp::class_type type) -> Ref<Component>
{
    if (!HasComponent(type))
        return nullptr;
    auto component = m_Components[type.get_hash()];
    return component;
}

void Entity::RemoveComponent(meta_hpp::class_type type)
{
    VERIFY(type.is_derived_from(meta_hpp::resolve_type<Component>()),
        "Attempted to remove a component that doesn't derive from Component, weird.");

    m_RemovedComponents.push_back(type.get_hash());
}

void Entity::OnDraw(RenderContext& context)
{
    ZoneScoped;
    for (auto& [id, component] : m_Components) {
        (void)id;
        component->OnDraw(context);
    }
}

void Entity::OnDestroy()
{
    LOG_DEBUGF("Destroying entity '{}'", Name);
    if (auto parent = m_Scene->GetEntity(m_Parent)) {
        parent->RemoveChild(*this);
    }
    // m_Scene->Destroy()
}

void Entity::OnUpdate(f32 const delta)
{
    if (m_Enabled) {
        for (auto& component : m_Components | std::views::values) {
            component->OnUpdate(delta);
        }
    }

    for (auto const& id : m_RemovedComponents) {
        m_Components.erase(id);
    }
    m_RemovedComponents.clear();
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
