#include "e5pch.h"
#include "Scene.h"
#include "Entity.h"
#include "Components/BaseComponents.h"
#include "Engin5/Input/Keys.h"

namespace Engin5
{
    Entity* Scene::CreateEntity(std::string const& name)
    {
        UUID id{};

        m_Entities[id] = Entity(id, this);
        auto& entity = m_Entities[id];
        entity.m_Name = name;
        return &entity;
    }

    Entity* Scene::GetEntity(UUID const handle)
    {
        return &m_Entities[handle];
    }
}