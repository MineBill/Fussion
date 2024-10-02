#include "Scene.h"

#include "Components/BaseComponents.h"
#include "Entity.h"
#include "Fussion/Input/Keys.h"
#include "FussionPCH.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    Scene::Scene()
    {
        m_name = "Cool Scene";
        m_entities[EntityHandle(0)] = Entity(EntityHandle(0), this);
        auto& root = m_entities[EntityHandle(0)];
        root.name = "Root";
    }

    Scene::Scene(Scene const& other)
        : Asset(other)
        , m_name(other.m_name)
        , m_entities(other.m_entities)
        , m_dirty(other.m_dirty)
    {
        m_handle = other.m_handle;
        for (auto& entity : m_entities | std::views::values) {
            entity.m_scene = this;

            for (auto& comp : entity.m_components | std::views::values) {
                comp->m_owner = &entity;
            }
        }
    }

    Scene::Scene(Scene&& other) noexcept
        : Asset(std::move(other))
        , m_name(std::move(other.m_name))
        , m_entities(std::move(other.m_entities))
        , m_dirty(other.m_dirty)
    {
        m_handle = other.m_handle;
        for (auto& entity : m_entities | std::views::values) {
            entity.m_scene = this;

            for (auto& comp : entity.m_components | std::views::values) {
                comp->m_owner = &entity;
            }
        }
    }

    Scene& Scene::operator=(Scene const& other)
    {
        if (this == &other)
            return *this;
        m_handle = other.m_handle;
        m_name = other.m_name;
        m_entities = other.m_entities;
        m_dirty = other.m_dirty;

        for (auto& entity : m_entities | std::views::values) {
            entity.m_scene = this;

            for (auto& comp : entity.m_components | std::views::values) {
                comp->m_owner = &entity;
            }
        }
        return *this;
    }

    Scene& Scene::operator=(Scene&& other) noexcept
    {
        if (this == &other)
            return *this;
        m_handle = other.m_handle;
        m_name = std::move(other.m_name);
        m_entities = std::move(other.m_entities);
        m_dirty = other.m_dirty;

        for (auto& entity : m_entities | std::views::values) {
            entity.m_scene = this;

            for (auto& comp : entity.m_components | std::views::values) {
                comp->m_owner = &entity;
            }
        }
        return *this;
    }

    void Scene::on_start()
    {
        for (auto& [id, entity] : m_entities) {
            entity.on_start();
        }
    }

    void Scene::on_update(f32 delta)
    {
        for (auto& [id, entity] : m_entities) {
            (void)id;
            entity.on_update(delta);
        }
    }

    void Scene::tick()
    {
        for (auto& [id, entity] : m_entities) {
            (void)id;
            entity.tick();
        }
    }

    void Scene::on_debug_draw(DebugDrawContext& ctx)
    {
        m_box =  BoundingBox();
        for (auto& [id, entity] : m_entities) {
            (void)id;
            entity.on_debug_draw(ctx);
        }
        Debug::draw_box(m_box, 0.0f, Color::Pink);
    }

    auto Scene::create_entity(std::string const& name, EntityHandle parent) -> Entity*
    {
        return create_entity_with_id(EntityHandle(), name, parent);
    }

    auto Scene::create_entity_with_id(EntityHandle id, std::string const& name, EntityHandle parent) -> Entity*
    {
        VERIFY(m_entities.contains(parent), "Parent {} is not a valid entity within the scene", parent);
        LOG_INFOF("Creating entity {} with parent {}", CAST(u64, id), CAST(u64, parent));

        m_entities[id] = Entity(id, this);
        auto& entity = m_entities[id];
        entity.name = name;

        auto local_id = m_local_id_counter++;
        entity.m_local_id = local_id;
        m_local_id_to_entity[local_id] = id;

        entity.set_parent(m_entities[parent]);
        return &entity;
    }

    auto Scene::get_entity(EntityHandle const handle) -> Entity*
    {
        if (!has_entity(handle))
            return nullptr;
        return &m_entities[handle];
    }

    auto Scene::get_entity_from_local_id(s32 local_id) -> Entity*
    {
        if (!m_local_id_to_entity.contains(local_id))
            return nullptr;
        return get_entity(m_local_id_to_entity[local_id]);
    }

    bool Scene::has_entity(EntityHandle handle) const
    {
        return m_entities.contains(handle);
    }

    void Scene::destroy(EntityHandle handle)
    {
        if (!has_entity(handle))
            return;

        for (auto child : m_entities[handle].m_children) {
            destroy(child);
        }

        m_entities[handle].on_destroy();
        m_entities.erase(handle);
    }

    void Scene::destroy(Entity const& entity)
    {
        destroy(entity.handle());
    }

    EntityHandle Scene::clone_entity(EntityHandle handle)
    {
        if (!m_entities.contains(handle)) {
            return EntityHandle::Invalid;
        }

        auto const& entity = m_entities[handle];

        auto new_entity = create_entity(entity.name, entity.m_parent);
        new_entity->transform = entity.transform;
        new_entity->m_enabled = entity.m_enabled;

        for (auto const& [component_id, component] : entity.get_components()) {
            (void)component_id;
            new_entity->add_component(component->clone());
        }

        // Transform = std::move(other.Transform);
        // Name = std::move(other.Name);
        // m_Parent = other.m_Parent;
        // m_Children = std::move(other.m_Children);
        // m_Components = std::move(other.m_Components);
        // m_RemovedComponents = std::move(other.m_RemovedComponents);
        // m_Handle = other.m_Handle;
        // m_Scene = other.m_Scene;
        // m_Enabled = other.m_Enabled;
        // m_LocalID = other.m_LocalID;

        return new_entity->m_handle;
    }

    void Scene::serialize(Serializer& ctx) const
    {
        FSN_SERIALIZE_MEMBER(m_name);
        ctx.begin_array("Entities", m_entities.size());
        for (auto const& [id, entity] : m_entities) {
            if (id == 0)
                continue;
            ctx.write("", entity);
        }
        ctx.end_array();
    }

    void Scene::deserialize(Deserializer& ctx)
    {
        FSN_DESERIALIZE_MEMBER(m_name);
        size_t size;
        ctx.begin_array("Entities", size);
        m_entities.reserve(size);

        // Because an entity's parent might not exist when deserializing
        // the entity, we defer the parenting operations until all the
        // entities are loaded.
        struct ParentChildPair {
            EntityHandle Parent {}, Child {};
        };
        std::vector<ParentChildPair> entities_to_resolve {};

        for (size_t i = 0; i < size; i++) {
            Entity e;
            e.m_scene = this;
            ctx.read("", e);

            if (!has_entity(e.m_parent)) {
                entities_to_resolve.emplace_back(e.m_parent, e.m_handle);
            } else {
                e.set_parent(m_entities[e.m_parent]);
            }
            auto local_id = m_local_id_counter++;
            e.m_local_id = local_id;
            m_local_id_to_entity[local_id] = e.m_handle;

            m_entities[e.m_handle] = std::move(e);
        }

        // Resolve all parent/child relationships.
        for (auto const& [parent, child] : entities_to_resolve) {
            get_entity(child)->set_parent(*get_entity(parent));
        }
        ctx.end_array();
    }

    auto Scene::root() -> Entity&
    {
        return m_entities[EntityHandle(0)];
    }
}
