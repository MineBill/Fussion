#pragma once
#include <Fussion/Assets/AssetRef.h>
#include <Fussion/Assets/Model.h>
#include <Fussion/Assets/PbrMaterial.h>
#include <Fussion/Reflection/PrivateReflection.h>
#include <Fussion/Scene/Component.h>

namespace Fussion {
    class [[API]] MeshRenderer final : public Component {
        FSN_ENABLE_PRIVATE_REFLECTION;

    public:
        COMPONENT_DEFAULT(MeshRenderer)

        virtual void OnStart() override;
        virtual void OnUpdate(f32 delta) override;
        virtual void OnDraw(RenderContext& ctx) override;

#if FSN_DEBUG_DRAW
        virtual void OnDebugDraw(DebugDrawContext& ctx) override;
#endif

        [[API, EditorName("Model")]]
        AssetRef<Model> ModelAsset;

        std::vector<AssetRef<PbrMaterial>> Materials {};

        virtual auto Clone() -> Ref<Component> override;

        virtual void Serialize(Serializer& ctx) const override;
        virtual void Deserialize(Deserializer& ctx) override;

    private:
        [[API, NotifyFor("Model")]]
        void OnModelChanged();
    };
}
