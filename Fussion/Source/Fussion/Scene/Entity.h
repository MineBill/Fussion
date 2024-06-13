#pragma once
#include "Component.h"
#include "Fussion/Core/Core.h"
#include "Fussion/Core/UUID.h"
#include "Generated/Entity_reflect_generated.h"

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

    REFLECT_CLASS()
    class Entity: REFLECT_BASE
    {
        friend class Scene;
        friend SceneSerializer;
        friend class SceneBinarySerializer;

        REFLECT_GENERATED_BODY()
    public:
        Transform Transform;

        Entity() = default;
        Entity(UUID const handle, Scene* scene): m_Handle(handle), m_Scene(scene) {}

        REFLECT_PROPERTY()
        void SetParent(Entity const& parent);

        REFLECT_PROPERTY()
        void AddChild(Entity const& child);

        REFLECT_PROPERTY()
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

        void AddComponent(Reflect::TypeId const& type_id);

        REFLECT_PROPERTY()
        UUID GetId() const { return m_Handle; }

        REFLECT_PROPERTY()
        std::string const& GetName() const { return m_Name; }

        REFLECT_PROPERTY()
        std::string& GetNameRef() { return m_Name; }

        std::map<UUID, Ref<Component>>& GetComponents() { return m_Components; }

        template<std::derived_from<Component> C>
        bool HasComponent() const
        {
            auto const id = C::GetStaticTypeInfo().GetTypeId().GetHash();
            return m_Components.contains(id);
        }

        REFLECT_PROPERTY()
        bool HasComponent(Reflect::TypeId const& type_id) const;

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
