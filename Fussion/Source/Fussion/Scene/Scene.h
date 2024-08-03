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

    void OnStart();
    void OnUpdate(f32 delta);

    void Tick();
#if USE_DEBUG_DRAW
    void OnDebugDraw();
#endif

    auto CreateEntity(std::string const& name = "Entity", Uuid parent = Uuid(0)) -> Entity*;
    auto CreateEntityWithId(Uuid id, std::string const& name = "Entity", Uuid parent = Uuid(0)) -> Entity*;

    auto GetEntity(Uuid handle) -> Entity*;
    auto GetEntity(Entity const& entity) -> Entity*;
    auto GetEntityFromLocalID(s32 local_id) -> Entity*;

    auto GetRoot() -> Entity*;

    template<typename Callback>
    void ForEachEntity(Callback callback)
    {
        for (auto&& [id, entity] : m_Entities) {
            callback(&entity);
        }
    }

    /// @return If the given handle is entity that exists or not in the scene.
    [[nodiscard]]
    bool IsHandleValid(Uuid parent) const;

    /// Marks the scene as modified.
    /// Mainly used by the editor.
    void SetDirty(bool dirty = true) { m_Dirty = dirty; }
    bool IsDirty() const { return m_Dirty; }

    void Destroy(Uuid handle);
    void Destroy(Entity const* entity);

    static auto GetStaticType() -> AssetType { return AssetType::Scene; }
    auto GetType() const -> AssetType override { return GetStaticType(); }

private:
    std::string m_Name{};
    std::unordered_map<Uuid, Entity> m_Entities{};
    bool m_Dirty{};

    std::unordered_map<s32, Uuid> m_LocalIDToEntity{};
    // We set this to 1 so that the first has an id of 1 because
    // the object picking framebuffer is filled with 0s.
    s32 m_LocalIDCounter{1};

    friend class Entity;
    friend SceneSerializer;
    friend SceneBinarySerializer;
};
}

namespace Fsn = Fussion;
