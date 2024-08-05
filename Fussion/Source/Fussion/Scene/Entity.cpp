#include "e5pch.h"
#include "Entity.h"
#include "Fussion/Scene/Scene.h"

#include "Components/Camera.h"

#include <ranges>
#include <tracy/Tracy.hpp>

namespace Fussion {
    Entity::Entity(const Entity& other): Transform(other.Transform),
                                         Name(other.Name),
                                         m_Parent(other.m_Parent),
                                         m_Children(other.m_Children),
                                         m_Components(other.m_Components),
                                         m_RemovedComponents(other.m_RemovedComponents),
                                         m_Handle(other.m_Handle),
                                         m_Scene(other.m_Scene),
                                         m_Enabled(other.m_Enabled) {}

    Entity::Entity(Entity&& other) noexcept: Transform(std::move(other.Transform)),
                                             Name(std::move(other.Name)),
                                             m_Parent(std::move(other.m_Parent)),
                                             m_Children(std::move(other.m_Children)),
                                             m_Components(std::move(other.m_Components)),
                                             m_RemovedComponents(std::move(other.m_RemovedComponents)),
                                             m_Handle(std::move(other.m_Handle)),
                                             m_Scene(other.m_Scene),
                                             m_Enabled(other.m_Enabled) {}

    Entity& Entity::operator=(const Entity& other)
    {
        if (this == &other)
            return *this;
        Transform = other.Transform;
        Name = other.Name;
        m_Parent = other.m_Parent;
        m_Children = other.m_Children;
        m_Components = other.m_Components;
        m_RemovedComponents = other.m_RemovedComponents;
        m_Handle = other.m_Handle;
        m_Scene = other.m_Scene;
        m_Enabled = other.m_Enabled;
        return *this;
    }

    Entity& Entity::operator=(Entity&& other) noexcept
    {
        if (this == &other)
            return *this;
        Transform = std::move(other.Transform);
        Name = std::move(other.Name);
        m_Parent = std::move(other.m_Parent);
        m_Children = std::move(other.m_Children);
        m_Components = std::move(other.m_Components);
        m_RemovedComponents = std::move(other.m_RemovedComponents);
        m_Handle = std::move(other.m_Handle);
        m_Scene = other.m_Scene;
        m_Enabled = other.m_Enabled;
        return *this;
    }

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
        ZoneScoped;

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
        ZoneScoped;

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
        ZoneScoped;

        return m_Components.contains(type.get_hash());
    }

    auto Entity::GetComponent(meta_hpp::class_type type) -> Ref<Component>
    {
        ZoneScoped;

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

    void Entity::OnDraw(RHI::RenderContext& context)
    {
        ZoneScoped;
        for (auto& [id, component] : m_Components) {
            (void)id;
            component->OnDraw(context);
        }
    }

    void Entity::OnStart()
    {
        ZoneScoped;
        for (auto& [id, component] : m_Components) {
            component->OnStart();
        }
    }

    void Entity::OnDestroy()
    {
        ZoneScoped;

        LOG_DEBUGF("Destroying entity '{}'", Name);
        for (auto& [id, component] : m_Components) {
            component->OnDestroy();
        }

        if (auto parent = m_Scene->GetEntity(m_Parent)) {
            parent->RemoveChild(*this);
        }
    }

    void Entity::OnUpdate(f32 const delta)
    {
        ZoneScoped;

        if (m_Enabled) {
            for (auto& component : m_Components | std::views::values) {
                component->OnUpdate(delta);
            }
        }
    }

    void Entity::Tick()
    {
        ZoneScoped;

        for (auto const& id : m_RemovedComponents) {
            m_Components.erase(id);
        }
        m_RemovedComponents.clear();
    }

    void Entity::OnDebugDraw(DebugDrawContext& ctx)
    {
        if (m_Enabled) {
            for (auto& component : m_Components | std::views::values) {
                component->OnDebugDraw(ctx);
            }
        }
    }

    bool Entity::IsGrandchild(Uuid handle) const
    {
        ZoneScoped;

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
