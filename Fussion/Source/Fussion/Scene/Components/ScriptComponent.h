#pragma once
#include "Fussion/Scene/Component.h"
#include "Fussion/Scripting/ScriptAssembly.h"
#include "Fussion/Scripting/ScriptingEngine.h"

namespace Fussion {
    class [[API]] ScriptComponent final : public Component {
    public:
        COMPONENT_DEFAULT(ScriptComponent)

        virtual void on_start() override;
        virtual void on_update(f32) override;

        virtual void on_destroy() override;

        void test() const;

        std::string class_name{};

        virtual auto clone() -> Ref<Component> override;

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;

    private:
        ScriptClass* m_script_class{ nullptr };
        ScriptInstance m_instance{};
    };
}
