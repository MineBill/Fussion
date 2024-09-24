#pragma once
#include <Fussion/Scene/Component.h>
#include <Fussion/Core/Core.h>
#include <Fussion/Math/Vector3.h>
#include <Fussion/Scene/Forward.h>
#include <Fussion/Serialization/ISerializable.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

class SceneSerializer;

namespace Fussion {

    struct Transform final : ISerializable {

        Vector3 position{};
        Vector3 euler_angles{};
        Vector3 scale = Vector3(1, 1, 1);

        [[nodiscard]]
        Mat4 matrix() const;

        [[nodiscard]]
        Mat4 camera_matrix() const;

        [[nodiscard]]
        Vector3 forward() const;

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };

    class ReflectionRegistry;

    class Entity final : public ISerializable {
        META_HPP_ENABLE_POLY_INFO()
        friend Scene;
        friend SceneSerializer;
        friend ReflectionRegistry;

    public:
        Transform transform;
        std::string name{ "Entity" };

        Entity() = default;
        Entity(EntityHandle handle, Scene* scene): m_handle(CAST(u64, handle)), m_scene(scene) {}
        virtual ~Entity() override = default;

        Entity(Entity const& other);
        Entity(Entity&& other) noexcept;
        Entity& operator=(Entity const& other);
        Entity& operator=(Entity&& other) noexcept;

        void set_parent(Entity const& new_parent);
        auto parent() const -> Entity*;
        void add_child(Entity& other);

        auto world_matrix() const -> Mat4;
        auto local_matrix() const -> Mat4;

        void set_enabled(bool enabled);
        bool is_enabled() const;
        bool* set_enabled() { return &m_enabled; }

        auto add_component(Ref<Component> const& component) -> Ref<Component>;
        auto add_component(meta_hpp::class_type type) -> Ref<Component>;
        [[nodiscard]]
        auto has_component(meta_hpp::class_type type) const -> bool;
        [[nodiscard]]
        auto get_component(meta_hpp::class_type type) -> Ref<Component>;
        void remove_component(meta_hpp::class_type type);

        template<std::derived_from<Component> C>
        auto add_component() -> Ref<C>
        {
            return std::dynamic_pointer_cast<C>(add_component(meta_hpp::resolve_type<C>()));
        }

        template<std::derived_from<Component> C>
        [[nodiscard]]
        auto has_component() const -> bool
        {
            return m_components.contains(EntityHandle(meta_hpp::resolve_type<C>().get_hash()));
        }

        template<std::derived_from<Component> C>
        [[nodiscard]]
        auto get_component() -> Ref<C>
        {
            return std::dynamic_pointer_cast<C>(get_component(meta_hpp::resolve_type<C>()));
        }

        template<std::derived_from<Component> C>
        void remove_component()
        {
            remove_component(meta_hpp::resolve_type<C>());
        }

        [[nodiscard]]
        auto handle() const -> EntityHandle { return m_handle; }

        /// Returns the local id of the entity for the scene it is currently in.
        [[nodiscard]]
        auto scene_local_id() const -> s32 { return m_local_id; }

        [[nodiscard]]
        auto get_components() const -> std::map<EntityHandle, Ref<Component>> const& { return m_components; }

        [[nodiscard]]
        auto children() const -> std::vector<EntityHandle> const& { return m_children; }

        void on_draw(RenderContext& context);

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;

        Scene& scene() const { return *m_scene; }

    private:
        void on_start();
        void on_update(f32 delta);

        void tick();

#if FSN_DEBUG_DRAW
        void on_debug_draw(DebugDrawContext& ctx);
#endif

        void on_destroy();

        bool is_grandchild(EntityHandle handle) const;
        void add_child_internal(Entity const& child);
        void remove_child_internal(Entity const& child);

        EntityHandle m_parent;
        std::vector<EntityHandle> m_children{};

        std::map<EntityHandle, Ref<Component>> m_components;
        std::vector<EntityHandle> m_removed_components{};

        EntityHandle m_handle;
        Scene* m_scene{};
        s32 m_local_id{};

        bool m_enabled{ true };
    };
}
