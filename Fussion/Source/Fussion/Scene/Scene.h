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

        void on_start();
        void on_update(f32 delta);

        void tick();
#if FSN_DEBUG_DRAW
        void on_debug_draw(DebugDrawContext& ctx);
#endif

        [[nodiscard]]
        auto create_entity(std::string const& name = "Entity", EntityHandle parent = EntityHandle(0)) -> Entity*;
        [[nodiscard]]
        auto create_entity_with_id(EntityHandle id, std::string const& name = "Entity", EntityHandle parent = EntityHandle(0)) -> Entity*;

        [[nodiscard]]
        auto get_entity(EntityHandle handle) -> Entity*;
        [[nodiscard]]
        auto get_entity_from_local_id(s32 local_id) -> Entity*;

        /// Returns the invisible root entity. All other entities in the scene
        /// are children of this root entity.
        [[nodiscard]]
        auto root() -> Entity&;

        template<typename Callback>
        void for_each_entity(Callback callback)
        {
            for (auto&& [id, entity] : m_entities) {
                (void)id;
                callback(&entity);
            }
        }

        /// Looks for the first component of type C in any entity and returns it.
        /// @returns The component if it exists, nullptr otherwise.
        template<std::derived_from<Component> C>
        [[nodiscard]]
        auto find_first_component() -> Ref<C>
        {
            for (auto& [id, entity] : m_entities) {
                (void)id;
                if (entity.has_component<C>()) {
                    return entity.get_component<C>();
                }
            }
            return nullptr;
        }

        /// Returns if the entity exists in the scene.
        [[nodiscard]]
        bool has_entity(EntityHandle handle) const;

        /// Marks the scene as modified.
        /// Mainly used by the editor.
        void set_dirty(bool dirty = true) { m_dirty = dirty; }
        bool is_dirty() const { return m_dirty; }

        void destroy(EntityHandle handle);
        void destroy(Entity const& entity);

        EntityHandle clone_entity(EntityHandle handle);

        auto name() const -> std::string const& { return m_name; }

        virtual auto type() const -> AssetType override { return static_type(); }

        static auto static_type() -> AssetType { return AssetType::Scene; }

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;

    private:
        std::string m_name{};
        std::unordered_map<EntityHandle, Entity> m_entities{};
        bool m_dirty{};

        std::unordered_map<s32, EntityHandle> m_local_id_to_entity{};
        // We set this to 1 so that the first has an id of 1 because
        // the object picking framebuffer is filled with 0s.
        s32 m_local_id_counter{ 1 };

        friend class Entity;
        friend SceneSerializer;
        friend SceneBinarySerializer;
    };
}

namespace Fsn = Fussion;
