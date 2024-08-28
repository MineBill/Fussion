#include "FussionPCH.h"
#include "Entity.h"
#include "Fussion/Scene/Scene.h"
#include "Serialization/Serializer.h"

#include "Components/Camera.h"

#include <ranges>
#include <tracy/Tracy.hpp>

namespace Fussion {
    Mat4 Transform::GetMatrix() const
    {
        auto scale = glm::scale(Mat4(1.0), CAST(glm::vec3, Scale));
        auto rotation = glm::eulerAngleYXZ(
            glm::radians(EulerAngles.Y),
            glm::radians(EulerAngles.X),
            glm::radians(EulerAngles.Z));
        auto translation = glm::translate(Mat4(1.0), CAST(glm::vec3, Position));
        return translation * rotation * scale;
    }

    Mat4 Transform::GetCameraMatrix() const
    {
        auto rotation = glm::eulerAngleZXY(
            glm::radians(EulerAngles.Z),
            glm::radians(EulerAngles.X),
            glm::radians(EulerAngles.Y));
        auto translation = glm::translate(Mat4(1.0), CAST(glm::vec3, Position));
        return rotation * glm::inverse(translation);
    }

    Vector3 Transform::GetForward() const
    {
        auto rotation = glm::mat3(glm::eulerAngleYXZ(
            glm::radians(EulerAngles.Y),
            glm::radians(EulerAngles.X),
            glm::radians(EulerAngles.Z)));

        return rotation * Vector3::Forward;
    }

    void Transform::Serialize(Serializer& ctx) const
    {
        FSN_SERIALIZE_MEMBER(Position);
        FSN_SERIALIZE_MEMBER(EulerAngles);
        FSN_SERIALIZE_MEMBER(Scale);
    }

    void Transform::Deserialize(Deserializer& ctx)
    {
        FSN_DESERIALIZE_MEMBER(Position);
        FSN_DESERIALIZE_MEMBER(EulerAngles);
        FSN_DESERIALIZE_MEMBER(Scale);
    }

    Entity::Entity(Entity const& other): Transform(other.Transform),
                                         Name(other.Name),
                                         m_Parent(other.m_Parent),
                                         m_Children(other.m_Children),
                                         m_Components(std::move(other.m_Components)),
                                         m_RemovedComponents(other.m_RemovedComponents),
                                         m_Handle(other.m_Handle),
                                         m_Scene(other.m_Scene),
                                         m_LocalID(other.m_LocalID),
                                         m_Enabled(other.m_Enabled)
    {

        for (auto& [id, component] : other.m_Components) {
            (void)id;
            component->m_Owner = this;
        }
    }

    Entity::Entity(Entity&& other) noexcept: Transform(std::move(other.Transform)),
                                             Name(std::move(other.Name)),
                                             m_Parent(other.m_Parent),
                                             m_Children(std::move(other.m_Children)),
                                             m_Components(std::move(other.m_Components)),
                                             m_RemovedComponents(std::move(other.m_RemovedComponents)),
                                             m_Handle(other.m_Handle),
                                             m_Scene(other.m_Scene),
                                             m_Enabled(other.m_Enabled)
    {
        for (auto& [id, component] : m_Components) {
            (void)id;
            component->m_Owner = this;
        }
    }

    Entity& Entity::operator=(Entity const& other)
    {
        if (this == &other)
            return *this;
        Transform = other.Transform;
        Name = other.Name;
        m_Parent = other.m_Parent;
        // Since Entity only has a handle to it's parent,
        // and we copy that handle here, we don't have to
        // "update" the children to point to us, because they
        // already do.
        m_Children = other.m_Children;
        m_Components = other.m_Components;
        m_RemovedComponents = other.m_RemovedComponents;
        m_Handle = other.m_Handle;
        m_Scene = other.m_Scene;
        m_Enabled = other.m_Enabled;

        for (auto& [id, component] : m_Components) {
            (void)id;
            component->m_Owner = this;
        }
        return *this;
    }

    Entity& Entity::operator=(Entity&& other) noexcept
    {
        if (this == &other)
            return *this;
        Transform = std::move(other.Transform);
        Name = std::move(other.Name);
        m_Parent = other.m_Parent;
        m_Children = std::move(other.m_Children);
        m_Components = std::move(other.m_Components);
        m_RemovedComponents = std::move(other.m_RemovedComponents);
        m_Handle = other.m_Handle;
        m_Scene = other.m_Scene;
        m_Enabled = other.m_Enabled;
        m_LocalID = other.m_LocalID;

        for (auto& [id, component] : m_Components) {
            (void)id;
            component->m_Owner = this;
        }
        return *this;
    }

    void Entity::SetParent(Entity const& new_parent)
    {
        if (IsGrandchild(new_parent.m_Handle))
            return;

        if (auto p = m_Scene->GetEntity(m_Parent)) {
            p->RemoveChildInternal(*this);
        }
        m_Scene->GetEntity(new_parent.m_Handle)->AddChildInternal(*this);
        m_Parent = new_parent.m_Handle;
    }

    void Entity::AddChild(Entity& other)
    {
        other.SetParent(*this);
    }

    void Entity::AddChildInternal(Entity const& child)
    {
        m_Children.push_back(child.m_Handle);
    }

    void Entity::RemoveChildInternal(Entity const& child)
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
        // If the parent cannot be found in the scene then we are the root entity, which is always enabled.
        return true;
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
        m_Components[EntityHandle(type.get_hash())] = comp;
        return comp;
    }

    auto Entity::HasComponent(meta_hpp::class_type type) const -> bool
    {
        ZoneScoped;

        return m_Components.contains(EntityHandle(type.get_hash()));
    }

    auto Entity::GetComponent(meta_hpp::class_type type) -> Ref<Component>
    {
        ZoneScoped;

        if (!HasComponent(type))
            return nullptr;
        auto component = m_Components[EntityHandle(type.get_hash())];
        return component;
    }

    void Entity::RemoveComponent(meta_hpp::class_type type)
    {
        VERIFY(type.is_derived_from(meta_hpp::resolve_type<Component>()),
            "Attempted to remove a component that doesn't derive from Component, weird.");

        m_RemovedComponents.push_back(EntityHandle(type.get_hash()));
    }

    void Entity::OnDraw(RenderContext& context)
    {
        ZoneScoped;
        for (auto& [id, component] : m_Components) {
            (void)id;
            component->OnDraw(context);
        }
    }

    void Entity::Serialize(Serializer& ctx) const
    {
        FSN_SERIALIZE_MEMBER(Name);
        FSN_SERIALIZE_MEMBER(m_Enabled);
        FSN_SERIALIZE_MEMBER(m_Handle);
        FSN_SERIALIZE_MEMBER(m_Parent);
        FSN_SERIALIZE_MEMBER(Transform);

        ctx.BeginObject("Components", m_Components.size());
        for (auto const& [id, component] : m_Components) {
            (void)id;
            auto name = component->meta_poly_ptr().get_type().as_pointer().get_data_type().get_metadata().at("Name").as<std::string>();
            ctx.Write(name, *component);
        }
        ctx.EndObject();
    }

    void Entity::Deserialize(Deserializer& ctx)
    {
        FSN_DESERIALIZE_MEMBER(Name);
        FSN_DESERIALIZE_MEMBER(m_Enabled);
        FSN_DESERIALIZE_MEMBER(m_Handle);
        FSN_DESERIALIZE_MEMBER(m_Parent);
        FSN_DESERIALIZE_MEMBER(Transform);

        size_t size;
        ctx.BeginObject("Components", size);

        auto registry = meta_hpp::resolve_scope("Components");
        for (auto const& key : ctx.ReadKeys()) {
            if (auto klass = registry.get_typedef(key); klass.is_valid()) {
                auto component = AddComponent(klass.as_class());
                ctx.Read(key, *component);
            }
        }
        ctx.EndObject();
    }

    void Entity::OnStart()
    {
        ZoneScoped;
        for (auto& [id, component] : m_Components) {
            (void)id;
            component->OnStart();
        }
    }

    void Entity::OnDestroy()
    {
        ZoneScoped;

        LOG_DEBUGF("Destroying entity '{}'", Name);
        for (auto& [id, component] : m_Components) {
            (void)id;
            component->OnDestroy();
        }

        if (auto parent = m_Scene->GetEntity(m_Parent)) {
            parent->RemoveChildInternal(*this);
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

    bool Entity::IsGrandchild(EntityHandle handle) const
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
