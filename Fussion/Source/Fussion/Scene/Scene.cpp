#include "e5pch.h"
#include "Scene.h"
#include "Entity.h"
#include "Components/BaseComponents.h"
#include "Fussion/Input/Keys.h"

namespace Fussion {
Scene::Scene()
{
    m_Name = "Cool Scene";
    m_Entities[Uuid(0)] = Entity(0, this);
    auto root = m_Entities[0];
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
    }
}

Scene::Scene(Scene&& other) noexcept: Asset(std::move(other)),
                                      m_Name(std::move(other.m_Name)),
                                      m_Entities(std::move(other.m_Entities)),
                                      m_Dirty(other.m_Dirty)
{
    m_Handle = other.m_Handle;
    for (auto& entity : m_Entities | std::views::values) {
        entity.m_Scene = this;
    }
}

Scene& Scene::operator=(const Scene& other)
{
    if (this == &other)
        return *this;
    Asset::operator =(other);
    m_Handle = other.m_Handle;
    m_Name = other.m_Name;
    m_Entities = other.m_Entities;
    m_Dirty = other.m_Dirty;

    for (auto& entity : m_Entities | std::views::values) {
        entity.m_Scene = this;
    }
    return *this;
}

Scene& Scene::operator=(Scene&& other) noexcept
{
    if (this == &other)
        return *this;
    Asset::operator =(std::move(other));
    m_Handle = other.m_Handle;
    m_Name = std::move(other.m_Name);
    m_Entities = std::move(other.m_Entities);
    m_Dirty = other.m_Dirty;

    for (auto& entity : m_Entities | std::views::values) {
        entity.m_Scene = this;
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

void Scene::OnDebugDraw()
{
    for (auto& [id, entity] : m_Entities) {
        (void)id;
        entity.OnDebugDraw();
    }
}

auto Scene::CreateEntity(std::string const& name, Uuid parent) -> Entity*
{
    return CreateEntityWithId(Uuid(), name, parent);
}

auto Scene::CreateEntityWithId(Uuid id, std::string const& name, Uuid parent) -> Entity*
{
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

auto Scene::GetEntity(Uuid const handle) -> Entity*
{
    if (!IsHandleValid(handle))
        return nullptr;
    return &m_Entities[handle];
}

auto Scene::GetEntity(Entity const& entity) -> Entity*
{
    return GetEntity(entity.m_Handle);
}

auto Scene::GetEntityFromLocalID(s32 local_id) -> Entity*
{
    if (!m_LocalIDToEntity.contains(local_id)) return nullptr;
    return GetEntity(m_LocalIDToEntity[local_id]);
}

bool Scene::IsHandleValid(Uuid parent) const
{
    return m_Entities.contains(parent);
}

void Scene::Destroy(Uuid handle)
{
    if (!IsHandleValid(handle))
        return;

    for (auto child : m_Entities[handle].m_Children) {
        Destroy(child);
    }

    m_Entities[handle].OnDestroy();
    m_Entities.erase(handle);
}

void Scene::Destroy(Entity const* entity)
{
    Destroy(entity->GetId());
}

auto Scene::GetRoot() -> Entity*
{
    return &m_Entities[0];
}
}
