#pragma once
#include "Entity.h"
#include "Fussion/Core/UUID.h"
#include "Fussion/Assets/Asset.h"
#include "Vulkan/Common.h"

class SceneSerializer;
class SceneBinarySerializer;

namespace Fussion {
class Scene : public Asset {
public:
    Scene();

    void OnUpdate(f32 delta);

    Entity* CreateEntity(std::string const& name = "Entity", UUID parent = UUID(0));
    Entity* CreateEntityWithID(UUID id, std::string const& name = "Entity", UUID parent = UUID(0));

    Entity* GetEntity(UUID handle);

    Entity* GetEntity(Entity const& entity)
    {
        return GetEntity(entity.m_Handle);
    }

    Entity* GetRoot();

    template<typename Callback>
    void ForEachEntity(Callback callback)
    {
        for (auto&& [id, entity] : m_Entities) {
            callback(&entity);
        }
    }

    /// @return If the given handle is entity that exists or not in the scene.
    [[nodiscard]] bool IsHandleValid(UUID parent) const;

    /// Marks the scene as modified.
    /// Mainly used by the editor.
    void SetDirty() { m_Dirty = true; }

    void Destroy(UUID handle);
    void Destroy(Entity const* entity);

    static AssetType GetStaticType() { return AssetType::Scene; }

private:
    std::string m_Name{};
    std::unordered_map<UUID, Entity> m_Entities{};
    bool m_Dirty{};

    friend class Entity;
    friend SceneSerializer;
    friend SceneBinarySerializer;
};
}

namespace Fsn = Fussion;
