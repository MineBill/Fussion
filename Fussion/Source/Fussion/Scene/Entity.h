#pragma once
#include "Component.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/UUID.h"

class SceneSerializer;
namespace Fussion
{

    struct Transform
    {
        Vector3 Position{};
        Vector3 EulerAngles{};
        Vector3 Scale = Vector3(1, 1, 1);

        Mat4 GetView() const
        {
            return Mat4(1.0);
        }
    };

    class Scene;

    class Entity
    {
        friend class Scene;
        friend SceneSerializer;
        friend class SceneBinarySerializer;

    public:
        Transform Transform;

        Entity() = default;
        Entity(UUID const handle, Scene* scene): m_Handle(handle), m_Scene(scene) {}

        void SetParent(Entity const& parent);

        void AddChild(Entity const& child);

        void RemoveChild(Entity const& child);

        template<std::derived_from<Component> C>
        Ref<C> AddComponent()
        {
            if (HasComponent<C>())
                return nullptr;
            auto const id = C::GetStaticTypeInfo().GetTypeId().GetHash();
            auto component = MakeRef<C>(this);
            m_Components[id] = component;
            return component;
        }

        // void AddComponent(Reflect::TypeId const& type_id);

        UUID GetId() const { return m_Handle; }

        std::string const& GetName() const { return m_Name; }

        std::string& GetNameRef() { return m_Name; }

        std::map<UUID, Ref<Component>>& GetComponents() { return m_Components; }

        template<std::derived_from<Component> C>
        bool HasComponent() const
        {
            auto const id = C::GetStaticTypeInfo().GetTypeId().GetHash();
            return m_Components.contains(id);
        }

        // bool HasComponent(Reflect::TypeId const& type_id) const;

    private:
        void OnUpdate(f32 delta);

        UUID m_Parent;
        std::vector<UUID> m_Children;

        std::map<UUID, Ref<Component>> m_Components;

        UUID m_Handle;
        std::string m_Name{"Entity"};
        Scene* m_Scene{};
    };
}
