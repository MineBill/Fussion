#include "FussionPCH.h"
#include "Scene.h"

#include "Components/BaseComponents.h"
#include "Entity.h"
#include "Fussion/Input/Keys.h"
#include "Serialization/Serializer.h"

namespace Fussion {
    Scene::Scene()
    {
        m_Name = "Cool Scene";
        m_Entities[EntityHandle(0)] = Entity(EntityHandle(0), this);
        auto& root = m_Entities[EntityHandle(0)];
        root.Name = "Root";
    }

    Scene::Scene(Scene const& other)
        : Asset(other)
        , m_Name(other.m_Name)
        , m_Entities(other.m_Entities)
        , m_Dirty(other.m_Dirty)
    {
        m_Handle = other.m_Handle;
        for (auto& entity : m_Entities | std::views::values) {
            entity.m_Scene = this;

            for (auto& comp : entity.m_Components | std::views::values) {
                comp->m_Owner = &entity;
            }
        }
    }

    Scene::Scene(Scene&& other) noexcept
        : Asset(std::move(other))
        , m_Name(std::move(other.m_Name))
        , m_Entities(std::move(other.m_Entities))
        , m_Dirty(other.m_Dirty)
    {
        m_Handle = other.m_Handle;
        for (auto& entity : m_Entities | std::views::values) {
            entity.m_Scene = this;

            for (auto& comp : entity.m_Components | std::views::values) {
                comp->m_Owner = &entity;
            }
        }
    }

    Scene& Scene::operator=(Scene const& other)
    {
        if (this == &other)
            return *this;
        m_Handle = other.m_Handle;
        m_Name = other.m_Name;
        m_Entities = other.m_Entities;
        m_Dirty = other.m_Dirty;

        for (auto& entity : m_Entities | std::views::values) {
            entity.m_Scene = this;

            for (auto& comp : entity.m_Components | std::views::values) {
                comp->m_Owner = &entity;
            }
        }
        return *this;
    }

    Scene& Scene::operator=(Scene&& other) noexcept
    {
        if (this == &other)
            return *this;
        m_Handle = other.m_Handle;
        m_Name = std::move(other.m_Name);
        m_Entities = std::move(other.m_Entities);
        m_Dirty = other.m_Dirty;

        for (auto& entity : m_Entities | std::views::values) {
            entity.m_Scene = this;

            for (auto& comp : entity.m_Components | std::views::values) {
                comp->m_Owner = &entity;
            }
        }
        return *this;
    }

    void Scene::OnStart()
    {
        for (auto& [id, entity] : m_Entities) {
            entity.OnStart();
        }
    }

    void Scene::OnUpdate(f32 delta)
    {
        for (auto& [id, entity] : m_Entities) {
            (void)id;
            entity.OnUpdate(delta);
        }
    }

    void Scene::Tick()
    {
        for (auto& [id, entity] : m_Entities) {
            (void)id;
            entity.Tick();
        }
    }

    void Scene::OnDebugDraw(DebugDrawContext& ctx)
    {
        m_Box = BoundingBox();
        for (auto& [id, entity] : m_Entities) {
            (void)id;
            entity.OnDebugDraw(ctx);
        }
        Debug::DrawBox(m_Box, 0.0f, Color::Pink);
    }

    auto Scene::CreateEntity(std::string const& name, EntityHandle parent) -> Entity*
    {
        return CreateEntityWithID(EntityHandle(), name, parent);
    }

    auto Scene::CreateEntityWithID(EntityHandle id, std::string const& name, EntityHandle parent) -> Entity*
    {
        VERIFY(m_Entities.contains(parent), "Parent {} is not a valid entity within the scene", parent);
        LOG_INFOF("Creating entity {} with parent {}", CAST(u64, id), CAST(u64, parent));

        m_Entities[id] = Entity(id, this);
        auto& entity = m_Entities[id];
        entity.Name = name;

        auto local_id = m_LocalIDCounter++;
        entity.m_LocalID = local_id;
        m_LocalIDToEntity[local_id] = id;

        entity.SetParent(m_Entities[parent]);
        return &entity;
    }

    auto Scene::GetEntity(EntityHandle const handle) -> Entity*
    {
        if (!HasEntity(handle))
            return nullptr;
        return &m_Entities[handle];
    }

    auto Scene::GetEntityFromLocalID(s32 local_id) -> Entity*
    {
        if (!m_LocalIDToEntity.contains(local_id))
            return nullptr;
        return GetEntity(m_LocalIDToEntity[local_id]);
    }

    bool Scene::HasEntity(EntityHandle handle) const
    {
        return m_Entities.contains(handle);
    }

    void Scene::DestroyEntity(EntityHandle handle)
    {
        if (!HasEntity(handle))
            return;

        for (auto child : m_Entities[handle].m_Children) {
            DestroyEntity(child);
        }

        m_Entities[handle].OnDestroy();
        m_Entities.erase(handle);
    }

    void Scene::DestroyEntity(Entity const& entity)
    {
        DestroyEntity(entity.GetHandle());
    }

    EntityHandle Scene::CloneEntity(EntityHandle handle)
    {
        if (!m_Entities.contains(handle)) {
            return EntityHandle::Invalid;
        }

        auto const& entity = m_Entities[handle];

        auto new_entity = CreateEntity(entity.Name, entity.m_Parent);
        new_entity->Transform = entity.Transform;
        new_entity->m_Enabled = entity.m_Enabled;

        for (auto const& [component_id, component] : entity.GetComponents()) {
            (void)component_id;
            new_entity->AddComponent(component->Clone());
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

        return new_entity->m_Handle;
    }

    void Scene::Serialize(Serializer& ctx) const
    {
        FSN_SERIALIZE_MEMBER(m_Name);
        ctx.BeginArray("Entities", m_Entities.size());
        for (auto const& [id, entity] : m_Entities) {
            if (id == 0)
                continue;
            ctx.Write("", entity);
        }
        ctx.EndArray();
    }

    void Scene::Deserialize(Deserializer& ctx)
    {
        FSN_DESERIALIZE_MEMBER(m_Name);
        size_t size;
        ctx.BeginArray("Entities", size);
        m_Entities.reserve(size);

        // Because an entity's parent might not exist when deserializing
        // the entity, we defer the parenting operations until all the
        // entities are loaded.
        struct ParentChildPair {
            EntityHandle Parent {}, Child {};
        };
        std::vector<ParentChildPair> entities_to_resolve {};

        for (size_t i = 0; i < size; i++) {
            Entity e;
            e.m_Scene = this;
            ctx.Read("", e);

            if (!HasEntity(e.m_Parent)) {
                entities_to_resolve.emplace_back(e.m_Parent, e.m_Handle);
            } else {
                e.SetParent(m_Entities[e.m_Parent]);
            }
            auto local_id = m_LocalIDCounter++;
            e.m_LocalID = local_id;
            m_LocalIDToEntity[local_id] = e.m_Handle;

            m_Entities[e.m_Handle] = std::move(e);
        }

        // Resolve all parent/child relationships.
        for (auto const& [parent, child] : entities_to_resolve) {
            GetEntity(child)->SetParent(*GetEntity(parent));
        }
        ctx.EndArray();
    }

    auto Scene::GetRoot() -> Entity&
    {
        return m_Entities[EntityHandle(0)];
    }
}
