#include "e5pch.h"
#include "Scene.h"
#include "Entity.h"
#include "Components/BaseComponents.h"
#include "Fussion/Input/Keys.h"

namespace Fussion
{
    Scene::Scene()
    {
        m_Name = "Cool Scene";
        m_Entities[UUID(0)] = Entity(0, this);
        auto root = m_Entities[0];
        root.m_Name = "Root";
    }

    void Scene::OnUpdate(f32 delta)
    {
        for (auto& [id, entity]: m_Entities) {
            entity.OnUpdate(delta);
        }
    }

    Entity* Scene::CreateEntity(std::string const& name, UUID parent)
    {
        return CreateEntityWithID(UUID(), name, parent);
    }

    Entity* Scene::CreateEntityWithID(UUID id, std::string const& name, UUID parent)
    {
        LOG_INFOF("Creating entity {} with parent {}", CAST(u64, id), CAST(u64, parent));
        m_Entities[id] = Entity(id, this);
        auto& entity = m_Entities[id];
        entity.m_Name = name;
        entity.SetParent(m_Entities[parent]);
        return &entity;
    }

    Entity* Scene::GetEntity(UUID const handle)
    {
        if (!IsHandleValid(handle))
            return nullptr;
        return &m_Entities[handle];
    }

    bool Scene::IsHandleValid(UUID parent) const
    {
        return m_Entities.contains(parent);
    }

    Entity* Scene::GetRoot()
    {
        return &m_Entities[0];
    }
}