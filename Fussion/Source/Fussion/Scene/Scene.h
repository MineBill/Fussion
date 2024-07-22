#pragma once
#include "Entity.h"
#include "Fussion/Core/Uuid.h"
#include "Fussion/Assets/Asset.h"
#include "Vulkan/Common.h"

#include <ranges>

class SceneSerializer;
class SceneBinarySerializer;

namespace Fussion {
class Scene : public Asset {
public:
    Scene();

    Scene(const Scene& other);
    Scene(Scene&& other) noexcept;
    Scene& operator=(const Scene& other);
    Scene& operator=(Scene&& other) noexcept;

    void OnUpdate(f32 delta);

    Entity* CreateEntity(std::string const& name = "Entity", Uuid parent = Uuid(0));
    Entity* CreateEntityWithId(Uuid id, std::string const& name = "Entity", Uuid parent = Uuid(0));

    Entity* GetEntity(Uuid handle);

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
    [[nodiscard]] bool IsHandleValid(Uuid parent) const;

    /// Marks the scene as modified.
    /// Mainly used by the editor.
    void SetDirty(bool dirty = true) { m_Dirty = dirty; }
    bool IsDirty() { return m_Dirty; }

    void Destroy(Uuid handle);
    void Destroy(Entity const* entity);

    static AssetType GetStaticType() { return AssetType::Scene; }
    AssetType GetType() const override { return GetStaticType(); }

private:
    std::string m_Name{};
    std::unordered_map<Uuid, Entity> m_Entities{};
    bool m_Dirty{};

    friend class Entity;
    friend SceneSerializer;
    friend SceneBinarySerializer;
};
}

namespace Fsn = Fussion;
