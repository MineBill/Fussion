#pragma once
#include "Core/Types.h"
#include "Scene/Components/ScriptComponent.h"

namespace Fussion {
    class Entity;

    class ScriptBase {
    public:
        explicit ScriptBase(Entity* owner);
        ScriptBase(Entity* owner, asIScriptObject* object);

        static ScriptBase* Create(Entity* owner);

#pragma region Script Override Methods
        void OnStart();
        void OnUpdate(f32 delta);
#pragma endregion

        Entity* GetOwner() const;

        void AddRef();
        void Release();

        ScriptBase& operator=(ScriptBase const& s);

    private:
        Entity* m_Owner{};
        asIScriptObject* m_ScriptObject{};

        u32 m_RefCount{ 1 };
    };
}
