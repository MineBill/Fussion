#pragma once
#include "Component.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/UUID.h"

class SceneSerializer;

namespace Fussion {

struct Transform {
    Vector3 Position{};
    Vector3 EulerAngles{};
    Vector3 Scale = Vector3(1, 1, 1);

    Mat4 GetView() const
    {
        return Mat4(1.0);
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

    auto AddComponent(meta_hpp::class_type type) -> Ref<Component>;

    [[nodiscard]]
    auto GetId() const -> UUID { return m_Handle; }

    [[nodiscard]]
    auto GetName() const -> std::string const& { return m_Name; }

    auto GetNameRef() -> std::string& { return m_Name; }

    [[nodicard]]
    auto GetComponents() const -> std::map<UUID, Ref<Component>> const& { return m_Components; }

    template<std::derived_from<Component> C>
    [[nodiscard]]
    auto HasComponent() const -> bool
    {
        return m_Components.contains(meta_hpp::resolve_type<C>().get_hash());
    }

    [[nodiscard]]
    auto HasComponent(meta_hpp::class_type type) const -> bool;

    template<std::derived_from<Component> C>
    [[nodiscard]]
    auto GetComponent() -> Ref<C>
    {
        VERIFY(HasComponent<C>());
        auto component = m_Components[meta_hpp::resolve_type<C>().get_hash()];
        return std::dynamic_pointer_cast<C>(component);
    }

    [[nodiscard]]
    auto GetChildren() const -> std::vector<UUID> const& { return m_Children; }

private:
    void OnUpdate(f32 delta);

    bool IsGrandchild(UUID handle) const;

    UUID m_Parent;
    std::vector<UUID> m_Children;

    std::map<UUID, Ref<Component>> m_Components;

    UUID m_Handle;
    std::string m_Name{ "Entity" };
    Scene* m_Scene{};
};
}
