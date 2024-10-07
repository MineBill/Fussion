#pragma once
#include <Fussion/Scene/Component.h>
#include <Fussion/Scripting/ScriptAssembly.h>
#include <Fussion/Scripting/ScriptingEngine.h>

namespace Fussion {
    class [[API]] ScriptComponent final : public Component {
    public:
        COMPONENT_DEFAULT(ScriptComponent);

        [[API]]
        std::string ClassName {};

        virtual void OnStart() override;
        virtual void OnUpdate(f32) override;

        virtual void OnDestroy() override;

        void test() const;

        virtual auto Clone() -> Ref<Component> override;

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

    private:
        ScriptInstance m_Instance {};
    };
}
