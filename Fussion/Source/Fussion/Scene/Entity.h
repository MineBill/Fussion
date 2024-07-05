#pragma once
#include "Component.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/UUID.h"
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

    Mat4 GetMatrix() const
    {
        auto scale = glm::scale(Mat4(1.0), CAST(glm::vec3, Scale));
        auto rotation = glm::eulerAngleZXY(
            glm::radians(EulerAngles.Z),
            glm::radians(EulerAngles.X),
            glm::radians(EulerAngles.Y));
        auto translation = glm::translate(Mat4(1.0), CAST(glm::vec3, Position));
        return translation * rotation * scale;
    }
};

class Scene;

class Entity {
    friend class Scene;
    friend SceneSerializer;
    friend class SceneBinarySerializer;

public:
    Transform Transform;

    Entity() = default;
    Entity(UUID const handle, Scene* scene): m_Handle(handle), m_Scene(scene) {}

    void SetParent(Entity const& new_parent);
    void AddChild(Entity const& child);
    void RemoveChild(Entity const& child);

    auto AddComponent(meta_hpp::class_type type) -> Ref<Component>;

    template<std::derived_from<Component> C>
    auto AddComponent() -> Ref<C>
    {
        auto type = meta_hpp::resolve_type<C>();
        if (HasComponent<C>()) {
            LOG_WARNF("Attempted to re-add component `{}`", type.get_metadata().at("Name").template as<std::string>());
            return nullptr;
        }
        auto const id = type.get_hash();
        m_Components[id] = MakeRef<C>(this);
        return m_Components[id];
    }

    [[nodiscard]]
    auto HasComponent(meta_hpp::class_type type) const -> bool;

    template<std::derived_from<Component> C>
    [[nodiscard]]
    auto HasComponent() const -> bool
    {
        return m_Components.contains(meta_hpp::resolve_type<C>().get_hash());
    }

    auto GetComponent(meta_hpp::class_type type) -> Ref<Component>;

    template<std::derived_from<Component> C>
    [[nodiscard]]
    auto GetComponent() -> Ref<C>
    {
        return std::dynamic_pointer_cast<C>(GetComponent(meta_hpp::resolve_type<C>()));
    }

    void RemoveComponent(meta_hpp::class_type type);

    template<std::derived_from<Component> C>
    void RemoveComponent()
    {
        RemoveComponent(meta_hpp::resolve_type<C>());
    }

    [[nodiscard]]
    auto GetId() const -> UUID { return m_Handle; }

    [[nodiscard]]
    auto GetName() const -> std::string const& { return m_Name; }

    auto GetNameRef() -> std::string& { return m_Name; }

    [[nodiscard]]
    auto GetComponents() const -> std::map<UUID, Ref<Component>> const& { return m_Components; }

    [[nodiscard]]
    auto GetChildren() const -> std::vector<UUID> const& { return m_Children; }

    void OnDraw(RenderContext& context);

private:
    void OnUpdate(f32 delta);
    void OnDestroy();

    bool IsGrandchild(UUID handle) const;

    UUID m_Parent;
    std::vector<UUID> m_Children{};

    std::map<UUID, Ref<Component>> m_Components{};
    std::vector<UUID> m_RemovedComponents{};

    UUID m_Handle;
    std::string m_Name{ "Entity" };
    Scene* m_Scene{};
};
}
