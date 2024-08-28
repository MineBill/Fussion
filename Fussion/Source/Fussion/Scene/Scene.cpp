#include "FussionPCH.h"
#include "Scene.h"
#include "Entity.h"
#include "Components/BaseComponents.h"
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

    Scene::Scene(const Scene& other): Asset(other),
                                      m_Name(other.m_Name),
                                      m_Entities(other.m_Entities),
                                      m_Dirty(other.m_Dirty)
    {
        m_Handle = other.m_Handle;
        for (auto& entity : m_Entities | std::views::values) {
            entity.m_Scene = this;

            for (auto& comp : entity.m_Components | std::views::values) {
                comp->m_Owner = &entity;
            }
        }
    }

    Scene::Scene(Scene&& other) noexcept:
        m_Name(std::move(other.m_Name)),
        m_Entities(std::move(other.m_Entities)),
        m_Dirty(other.m_Dirty),
        Asset(std::move(other))
    {
        m_Handle = other.m_Handle;
        for (auto& entity : m_Entities | std::views::values) {
            entity.m_Scene = this;

            for (auto& comp : entity.m_Components | std::views::values) {
                comp->m_Owner = &entity;
            }
        }
    }

    Scene& Scene::operator=(const Scene& other)
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
        for (auto& [id, entity] : m_Entities) {
            (void)id;
            entity.OnDebugDraw(ctx);
        }
    }

    auto Scene::CreateEntity(std::string const& name, EntityHandle parent) -> Entity*
    {
        return CreateEntityWithId(EntityHandle(), name, parent);
    }

    auto Scene::CreateEntityWithId(EntityHandle id, std::string const& name, EntityHandle parent) -> Entity*
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

    void Scene::Destroy(EntityHandle handle)
    {
        if (!HasEntity(handle))
            return;

        for (auto child : m_Entities[handle].m_Children) {
            Destroy(child);
        }

        m_Entities[handle].OnDestroy();
        m_Entities.erase(handle);
    }

    void Scene::Destroy(Entity const& entity)
    {
        Destroy(entity.GetId());
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
            EntityHandle Parent{}, Child{};
        };
        std::vector<ParentChildPair> entities_to_resolve{};

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
