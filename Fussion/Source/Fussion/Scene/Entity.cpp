#include "Entity.h"

#include "Components/Camera.h"
#include "Fussion/Scene/Scene.h"
#include "FussionPCH.h"
#include "Serialization/Serializer.h"

#include <ranges>
#include <tracy/Tracy.hpp>

namespace Fussion {
    Mat4 Transform::matrix() const
    {
        auto scale_mat = glm::scale(Mat4(1.0), CAST(glm::vec3, this->scale));
        auto translation_mat = glm::translate(Mat4(1.0), CAST(glm::vec3, position));
        return translation_mat * rotation() * scale_mat;
    }

    Mat4 Transform::rotation() const
    {
        return glm::eulerAngleYXZ(
            glm::radians(euler_angles.y),
            glm::radians(euler_angles.x),
            glm::radians(euler_angles.z));
    }

    Mat4 Transform::camera_matrix() const
    {
        auto translation_mat = glm::translate(Mat4(1.0), CAST(glm::vec3, position));
        return rotation() * glm::inverse(translation_mat);
    }

    Vector3 Transform::forward() const
    {
        auto rotation_mat = glm::mat3(glm::eulerAngleYXZ(
            glm::radians(euler_angles.y),
            glm::radians(euler_angles.x),
            glm::radians(euler_angles.z)));

        return rotation_mat * Vector3::Forward;
    }

    void Transform::serialize(Serializer& ctx) const
    {
        FSN_SERIALIZE_MEMBER(position);
        FSN_SERIALIZE_MEMBER(euler_angles);
        FSN_SERIALIZE_MEMBER(scale);
    }

    void Transform::deserialize(Deserializer& ctx)
    {
        FSN_DESERIALIZE_MEMBER(position);
        FSN_DESERIALIZE_MEMBER(euler_angles);
        FSN_DESERIALIZE_MEMBER(scale);
    }

    Entity::Entity(Entity const& other)
        : transform(other.transform)
        , name(other.name)
        , m_parent(other.m_parent)
        , m_children(other.m_children)
        , m_components(std::move(other.m_components))
        , m_removed_components(other.m_removed_components)
        , m_handle(other.m_handle)
        , m_scene(other.m_scene)
        , m_local_id(other.m_local_id)
        , m_enabled(other.m_enabled)
    {

        for (auto& [id, component] : other.m_components) {
            (void)id;
            component->m_owner = this;
        }
    }

    Entity::Entity(Entity&& other) noexcept
        : transform(std::move(other.transform))
        , name(std::move(other.name))
        , m_parent(other.m_parent)
        , m_children(std::move(other.m_children))
        , m_components(std::move(other.m_components))
        , m_removed_components(std::move(other.m_removed_components))
        , m_handle(other.m_handle)
        , m_scene(other.m_scene)
        , m_enabled(other.m_enabled)
    {
        for (auto& [id, component] : m_components) {
            (void)id;
            component->m_owner = this;
        }
    }

    Entity& Entity::operator=(Entity const& other)
    {
        if (this == &other)
            return *this;
        transform = other.transform;
        name = other.name;
        m_parent = other.m_parent;
        // Since Entity only has a handle to it's parent,
        // and we copy that handle here, we don't have to
        // "update" the children to point to us, because they
        // already do.
        m_children = other.m_children;
        m_components = other.m_components;
        m_removed_components = other.m_removed_components;
        m_handle = other.m_handle;
        m_scene = other.m_scene;
        m_enabled = other.m_enabled;

        for (auto& [id, component] : m_components) {
            (void)id;
            component->m_owner = this;
        }
        return *this;
    }

    Entity& Entity::operator=(Entity&& other) noexcept
    {
        if (this == &other)
            return *this;
        transform = std::move(other.transform);
        name = std::move(other.name);
        m_parent = other.m_parent;
        m_children = std::move(other.m_children);
        m_components = std::move(other.m_components);
        m_removed_components = std::move(other.m_removed_components);
        m_handle = other.m_handle;
        m_scene = other.m_scene;
        m_enabled = other.m_enabled;
        m_local_id = other.m_local_id;

        for (auto& [id, component] : m_components) {
            (void)id;
            component->m_owner = this;
        }
        return *this;
    }

    void Entity::set_parent(Entity const& new_parent)
    {
        if (is_grandchild(new_parent.m_handle))
            return;

        if (auto p = m_scene->get_entity(m_parent)) {
            p->remove_child_internal(*this);
        }
        m_scene->get_entity(new_parent.m_handle)->add_child_internal(*this);
        m_parent = new_parent.m_handle;
    }

    auto Entity::parent() const -> Entity*
    {
        return m_scene->get_entity(m_parent);
    }

    void Entity::add_child(Entity& other)
    {
        other.set_parent(*this);
    }

    auto Entity::world_matrix() const -> Mat4
    {
        if (auto parent = m_scene->get_entity(m_parent)) {
            return parent->world_matrix() * local_matrix();
        }
        return local_matrix();
    }

    auto Entity::local_matrix() const -> Mat4
    {
        return transform.matrix();
    }

    void Entity::add_child_internal(Entity const& child)
    {
        m_children.push_back(child.m_handle);
    }

    void Entity::remove_child_internal(Entity const& child)
    {
        ZoneScoped;

        if (auto const pos = std::ranges::find(m_children, child.m_handle); pos != std::end(m_children)) {
            std::erase(m_children, *pos);
        }
    }

    void Entity::set_enabled(bool enabled)
    {
        m_enabled = enabled;
        if (enabled) {
            for (auto& [id, component] : m_components) {
                (void)id;
                component->on_enabled();
            }
        } else {
            for (auto& [id, component] : m_components) {
                (void)id;
                component->on_disabled();
            }
        }
    }

    bool Entity::is_enabled() const
    {
        if (!m_enabled)
            return false;
        if (auto parent = m_scene->get_entity(m_parent)) {
            return parent->is_enabled();
        }
        // If the parent cannot be found in the scene then we are the root entity, which is always enabled.
        return true;
    }

    auto Entity::add_component(Ref<Component> const& component) -> Ref<Component>
    {
        ZoneScoped;
        auto const& meta = component->meta_poly_ptr();
        auto id = EntityHandle(meta.get_type().get_hash());
        if (m_components.contains(id)) {
            LOG_WARN("Component already exists on this entity");
            return nullptr;
        }
        component->m_owner = this;
        component->on_create();
        m_components[id] = component;
        return component;
    }

    auto Entity::add_component(meta_hpp::class_type type) -> Ref<Component>
    {
        ZoneScoped;

        VERIFY(type.is_derived_from(meta_hpp::resolve_type<Component>()),
            "Attempted to add a component that doesn't derive from Component, weird.");

        auto component = make_ref<Component>();

        auto data = type.create(this);

        auto ptr = *CAST(Component**, data.get_data());
        Ref<Component> comp;
        comp.reset(ptr);

        comp->on_create();
        m_components[EntityHandle(type.get_hash())] = comp;
        return comp;
    }

    auto Entity::has_component(meta_hpp::class_type type) const -> bool
    {
        ZoneScoped;

        return m_components.contains(EntityHandle(type.get_hash()));
    }

    auto Entity::get_component(meta_hpp::class_type type) -> Ref<Component>
    {
        ZoneScoped;

        if (!has_component(type))
            return nullptr;
        auto component = m_components[EntityHandle(type.get_hash())];
        return component;
    }

    void Entity::remove_component(meta_hpp::class_type type)
    {
        VERIFY(type.is_derived_from(meta_hpp::resolve_type<Component>()),
            "Attempted to remove a component that doesn't derive from Component, weird.");

        m_removed_components.push_back(EntityHandle(type.get_hash()));
    }

    void Entity::on_draw(RenderContext& context)
    {
        ZoneScoped;
        for (auto& [id, component] : m_components) {
            (void)id;
            component->on_draw(context);
        }
    }

    void Entity::serialize(Serializer& ctx) const
    {
        FSN_SERIALIZE_MEMBER(name);
        FSN_SERIALIZE_MEMBER(m_enabled);
        FSN_SERIALIZE_MEMBER(m_handle);
        FSN_SERIALIZE_MEMBER(m_parent);
        FSN_SERIALIZE_MEMBER(transform);

        ctx.begin_object("Components", m_components.size());
        for (auto const& [id, component] : m_components) {
            (void)id;
            auto component_name = component->meta_poly_ptr().get_type().as_pointer().get_data_type().get_metadata().at("Name").as<std::string>();
            ctx.write(component_name, *component);
        }
        ctx.end_object();
    }

    void Entity::deserialize(Deserializer& ctx)
    {
        FSN_DESERIALIZE_MEMBER(name);
        FSN_DESERIALIZE_MEMBER(m_enabled);
        FSN_DESERIALIZE_MEMBER(m_handle);
        FSN_DESERIALIZE_MEMBER(m_parent);
        FSN_DESERIALIZE_MEMBER(transform);

        size_t size;
        ctx.begin_object("Components", size);

        auto registry = meta_hpp::resolve_scope("Components");
        for (auto const& key : ctx.read_keys()) {
            if (auto klass = registry.get_typedef(key); klass.is_valid()) {
                auto component = add_component(klass.as_class());
                ctx.read(key, *component);
            }
        }
        ctx.end_object();
    }

    void Entity::on_start()
    {
        ZoneScoped;
        for (auto& [id, component] : m_components) {
            (void)id;
            component->on_start();
        }
    }

    void Entity::on_destroy()
    {
        ZoneScoped;

        LOG_DEBUGF("Destroying entity '{}'", name);
        for (auto& [id, component] : m_components) {
            (void)id;
            component->on_destroy();
        }

        if (auto parent = m_scene->get_entity(m_parent)) {
            parent->remove_child_internal(*this);
        }
    }

    void Entity::on_update(f32 const delta)
    {
        ZoneScoped;

        if (m_enabled) {
            for (auto& component : m_components | std::views::values) {
                component->on_update(delta);
            }
        }
    }

    void Entity::tick()
    {
        ZoneScoped;

        for (auto const& id : m_removed_components) {
            m_components.erase(id);
        }
        m_removed_components.clear();
    }

    void Entity::on_debug_draw(DebugDrawContext& ctx)
    {
        if (m_enabled) {
            for (auto& component : m_components | std::views::values) {
                component->on_debug_draw(ctx);
            }
        }
    }

    bool Entity::is_grandchild(EntityHandle handle) const
    {
        ZoneScoped;

        if (m_children.empty()) {
            return false;
        }

        for (auto const& child : m_children) {
            if (child == handle) {
                return true;
            }

            if (auto en = m_scene->get_entity(child); en->is_grandchild(handle)) {
                return true;
            }
        }
        return false;
    }
}
