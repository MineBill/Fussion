#pragma once
#include "Component.h"
#include "Engin5/Core/Core.h"
#include "Engin5/Core/UUID.h"

namespace Engin5
{
    class Scene;
    class Entity
    {
        friend class Scene;
    public:
        Entity() = default;
        Entity(UUID const handle, Scene* scene): m_Handle(handle), m_Scene(scene) {}

        void SetParent(Entity const& parent);
        void AddChild(Entity const& child);
        void RemoveChild(Entity const& child);

        template<std::derived_from<Component> C>
        Ref<C> AddComponent()
        {
            auto const id = C::GetStaticTypeInfo().GetTypeId().GetHash();
            auto component = MakeRef<C>(this);
            m_Components[id] = component;
            return component;
        }

        void AddComponent(Reflect::TypeId const& type_id);

        UUID GetId() const { return m_Handle; }
        std::string const& GetName() const { return m_Name; }
        std::string& GetNameRef() { return m_Name; }

        std::map<UUID, Ref<Component>>& GetComponents() { return m_Components; }
    private:
        UUID m_Parent;
        std::vector<UUID> m_Children;

        std::map<UUID, Ref<Component>> m_Components;

        UUID m_Handle;
        std::string m_Name{"Entity"};
        Scene* m_Scene{};
    };
}
