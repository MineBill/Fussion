#pragma once
#include "Component.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/Uuid.h"
#include "Fussion/Math/Vector3.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

class SceneSerializer;

namespace Fussion {

    struct Transform {
        Vector3 Position{};
        Vector3 EulerAngles{};
        Vector3 Scale = Vector3(1, 1, 1);

        Mat4 GetMatrix() const;

        Mat4 GetCameraMatrix() const;

        auto GetForward() const -> Vector3
        ;
    };

    class Scene;

    class Entity final {
        friend class Scene;
        friend SceneSerializer;
        friend class SceneBinarySerializer;
        friend class ReflRegistrar;

    public:
        Transform Transform;
        std::string Name{ "Entity" };

        Entity() = default;
        Entity(Uuid const handle, Scene* scene): m_Handle(handle), m_Scene(scene) {}

        Entity(Entity const& other);
        Entity(Entity&& other) noexcept;
        Entity& operator=(Entity const& other);
        Entity& operator=(Entity&& other) noexcept;

        void SetParent(Entity const& new_parent);

        void SetEnabled(bool enabled);
        bool IsEnabled() const;
        bool* GetEnabled() { return &m_Enabled; }

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
            return m_Components.contains(meta_hpp::resolve_type<C>().get_hash());
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
        auto GetId() const -> Uuid { return m_Handle; }

        [[nodiscard]]
        auto GetLocalID() const -> s32 { return m_LocalID; }

        [[nodiscard]]
        auto GetComponents() const -> std::map<Uuid, Ref<Component>> const& { return m_Components; }

        [[nodiscard]]
        auto GetChildren() const -> std::vector<Uuid> const& { return m_Children; }

        void OnDraw(RHI::RenderContext& context);

    private:
        void OnStart();
        void OnUpdate(f32 delta);

        void Tick();

#if FSN_DEBUG_DRAW
        void OnDebugDraw(DebugDrawContext& ctx);
#endif

        void OnDestroy();

        bool IsGrandchild(Uuid handle) const;
        void AddChild(Entity const& child);
        void RemoveChild(Entity const& child);

        Uuid m_Parent;
        std::vector<Uuid> m_Children{};

        std::map<Uuid, Ref<Component>> m_Components{};
        std::vector<Uuid> m_RemovedComponents{};

        Uuid m_Handle;
        Scene* m_Scene{};
        s32 m_LocalID{};

        bool m_Enabled{ true };
    };
}
