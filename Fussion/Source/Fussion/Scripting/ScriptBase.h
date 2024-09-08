#pragma once
#include "Core/Types.h"
#include "Scene/Components/ScriptComponent.h"

namespace Fussion {
    class Entity;

    class ScriptBase {
    public:
        explicit ScriptBase(Entity* owner);
        ScriptBase(Entity* owner, asIScriptObject* object);

        static ScriptBase* create(Entity* owner);

#pragma region Script Override Methods
        void on_start() const;
        void on_update(f32 delta);
#pragma endregion

        Entity* get_owner() const;

        void add_ref();
        void release();

        ScriptBase& operator=(ScriptBase const& s);

    private:
        Entity* m_owner{};
        asIScriptObject* m_script_object{};

        u32 m_ref_count{ 1 };
    };
}
