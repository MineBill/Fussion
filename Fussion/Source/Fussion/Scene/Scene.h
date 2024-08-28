#pragma once
#include <Fussion/Assets/Asset.h>
#include <Fussion/Scene/Entity.h>
#include <Fussion/Scene/Forward.h>

#include <ranges>

class SceneSerializer;
class SceneBinarySerializer;

namespace Fussion {
    class Scene final : public Asset {
    public:
        Scene();
        virtual ~Scene() override = default;

        Scene(Scene const& other);
        Scene(Scene&& other) noexcept;
        Scene& operator=(Scene const& other);
        Scene& operator=(Scene&& other) noexcept;

        void OnStart();
        void OnUpdate(f32 delta);

        void Tick();
#if FSN_DEBUG_DRAW
        void OnDebugDraw(DebugDrawContext& ctx);
#endif

        [[nodiscard]]
        auto CreateEntity(std::string const& name = "Entity", EntityHandle parent = EntityHandle(0)) -> Entity*;
        [[nodiscard]]
        auto CreateEntityWithId(EntityHandle id, std::string const& name = "Entity", EntityHandle parent = EntityHandle(0)) -> Entity*;

        [[nodiscard]]
        auto GetEntity(EntityHandle handle) -> Entity*;
        [[nodiscard]]
        auto GetEntityFromLocalID(s32 local_id) -> Entity*;

        /// Returns the invisible root entity. All other entities in the scene
        /// are children of this root entity.
        [[nodiscard]]
        auto GetRoot() -> Entity&;

        template<typename Callback>
        void ForEachEntity(Callback callback)
        {
            for (auto&& [id, entity] : m_Entities) {
                (void)id;
                callback(&entity);
            }
        }

        /// Looks for the first component of type C in any entity and returns it.
        /// @returns The component if it exists, nullptr otherwise.
        template<std::derived_from<Component> C>
        [[nodiscard]]
        auto FindFirstComponent() -> Ref<C>
        {
            for (auto& [id, entity] : m_Entities) {
                (void)id;
                if (entity.HasComponent<C>()) {
                    return entity.GetComponent<C>();
                }
            }
            return nullptr;
        }

        /// Returns if the entity exists in the scene.
        [[nodiscard]]
        bool HasEntity(EntityHandle handle) const;

        /// Marks the scene as modified.
        /// Mainly used by the editor.
        void SetDirty(bool dirty = true) { m_Dirty = dirty; }
        bool IsDirty() const { return m_Dirty; }

        void Destroy(EntityHandle handle);
        void Destroy(Entity const& entity);

        EntityHandle CloneEntity(EntityHandle handle);

        auto Name() const -> std::string const& { return m_Name; }

        virtual auto GetType() const -> AssetType override { return GetStaticType(); }

        static auto GetStaticType() -> AssetType { return AssetType::Scene; }

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

    private:
        std::string m_Name{};
        std::unordered_map<EntityHandle, Entity> m_Entities{};
        bool m_Dirty{};

        std::unordered_map<s32, EntityHandle> m_LocalIDToEntity{};
        // We set this to 1 so that the first has an id of 1 because
        // the object picking framebuffer is filled with 0s.
        s32 m_LocalIDCounter{ 1 };

        friend class Entity;
        friend SceneSerializer;
        friend SceneBinarySerializer;
    };
}

namespace Fsn = Fussion;
