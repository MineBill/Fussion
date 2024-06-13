#pragma once
#include "Entity.h"
#include "Engin5/Core/UUID.h"
#include "Engin5/Assets/Asset.h"

namespace Engin5
{
    class Scene: public Asset
    {
        friend class Entity;
        friend class SceneSerializer;
        friend class SceneBinarySerializer;
    public:
        Scene();

        void OnUpdate(f32 delta);

        Entity* CreateEntity(std::string const& name = "Entity");
        Entity* CreateEntityWithID(UUID id, std::string const& name = "Entity");

        Entity* GetEntity(UUID handle);
        Entity* GetEntity(Entity const& entity)
        {
            return GetEntity(entity.m_Handle);
        }

        template<typename Callback>
        void ForEachEntity(Callback callback)
        {
            for (auto&& [id, entity] : m_Entities) {
                callback(entity);
            }
        }

        static AssetType GetStaticType() { return AssetType::Scene; }
    private:
        std::string m_Name{};
        std::unordered_map<UUID, Entity> m_Entities{};
    };
}
