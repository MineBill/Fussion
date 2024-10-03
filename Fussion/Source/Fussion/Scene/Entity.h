#pragma once
#include <Fussion/Core/Core.h>
#include <Fussion/Math/Vector3.h>
#include <Fussion/Scene/Component.h>
#include <Fussion/Scene/Forward.h>
#include <Fussion/Serialization/ISerializable.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

class SceneSerializer;

namespace Fussion {

    struct Transform final : ISerializable {

        Vector3 Position {};
        Vector3 EulerAngles {};
        Vector3 Scale { 1, 1, 1 };

        [[nodiscard]]
        Mat4 Matrix() const;

        [[nodiscard]]
        Mat4 RotationMatrix() const;

        [[nodiscard]]
        Mat4 AsCameraMatrix() const;

        [[nodiscard]]
        Vector3 Forward() const;

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;
    };

    class ReflectionRegistry;

    class Entity final : public ISerializable {
        META_HPP_ENABLE_POLY_INFO()
        friend Scene;
        friend SceneSerializer;
        friend ReflectionRegistry;

    public:
        Transform Transform;
        std::string Name { "Entity" };

        Entity() = default;
        Entity(EntityHandle handle, Scene* scene)
            : m_Handle(CAST(u64, handle))
            , m_Scene(scene)
        { }
        virtual ~Entity() override = default;

        Entity(Entity const& other);
        Entity(Entity&& other) noexcept;
        Entity& operator=(Entity const& other);
        Entity& operator=(Entity&& other) noexcept;

        void SetParent(Entity const& new_parent);
        auto GetParent() const -> Entity*;
        void AddChild(Entity& other);

        auto WorldMatrix() const -> Mat4;
        auto LocalMatrix() const -> Mat4;

        void SetEnabled(bool enabled);
        bool IsEnabled() const;
        bool* SetEnabled() { return &m_Enabled; }

        auto AddComponent(Ref<Component> const& component) -> Ref<Component>;
        auto AddComponent(meta_hpp::class_type type) -> Ref<Component>;
        [[nodiscard]]
        auto HasComponent(meta_hpp::class_type type) const -> bool;
        [[nodiscard]]
        auto GetComponent(meta_hpp::class_type type) -> Ref<Component>;
        void RemoveComponent(meta_hpp::class_type type);

        template<std::derived_from<Component> C>
        auto AddComponent() -> Ref<C>
        {
            return std::dynamic_pointer_cast<C>(AddComponent(meta_hpp::resolve_type<C>()));
        }

        template<std::derived_from<Component> C>
        [[nodiscard]]
        auto HasComponent() const -> bool
        {
            return m_Components.contains(EntityHandle(meta_hpp::resolve_type<C>().get_hash()));
        }

        template<std::derived_from<Component> C>
        [[nodiscard]]
        auto GetComponent() -> Ref<C>
        {
            return std::dynamic_pointer_cast<C>(GetComponent(meta_hpp::resolve_type<C>()));
        }

        template<std::derived_from<Component> C>
        void RemoveComponent()
        {
            RemoveComponent(meta_hpp::resolve_type<C>());
        }

        [[nodiscard]]
        auto GetHandle() const -> EntityHandle
        {
            return m_Handle;
        }

        /// Returns the local id of the entity for the scene it is currently in.
        [[nodiscard]]
        auto SceneLocalID() const -> s32
        {
            return m_LocalID;
        }

        [[nodiscard]]
        auto GetComponents() const -> std::map<EntityHandle, Ref<Component>> const&
        {
            return m_Components;
        }

        [[nodiscard]]
        auto GetChildren() const -> std::vector<EntityHandle> const&
        {
            return m_Children;
        }

        void OnDraw(RenderContext& context);

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

        Scene& GetScene() const { return *m_Scene; }

        BoundingBox& GetBoundingBox() { return m_Box; }

    private:
        void OnStart();
        void OnUpdate(f32 delta);

        void Tick();

#if FSN_DEBUG_DRAW
        void OnDebugDraw(DebugDrawContext& ctx);
#endif

        void OnDestroy();

        bool IsGrandchild(EntityHandle handle) const;
        void AddChildInternal(Entity const& child);
        void RemoveChildInternal(Entity const& child);

        EntityHandle m_Parent;
        std::vector<EntityHandle> m_Children {};

        std::map<EntityHandle, Ref<Component>> m_Components;
        std::vector<EntityHandle> m_RemovedComponents {};

        EntityHandle m_Handle;
        Scene* m_Scene {};
        s32 m_LocalID {};

        BoundingBox m_Box {};
        bool m_Enabled { true };
    };
}
