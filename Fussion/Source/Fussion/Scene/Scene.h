#pragma once
#include "Entity.h"
#include "Fussion/Core/UUID.h"
#include "Fussion/Assets/Asset.h"

class SceneSerializer;
class SceneBinarySerializer;
namespace Fussion
{
    class Scene: public Asset
    {
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

        friend class Entity;
        friend SceneSerializer;
        friend SceneBinarySerializer;
    };
}

namespace Fsn = Fussion;
