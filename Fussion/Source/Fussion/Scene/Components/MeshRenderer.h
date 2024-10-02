#pragma once
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Assets/Model.h"
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Reflection/PrivateReflection.h"

namespace Fussion {
    class [[API]] MeshRenderer final : public Component {
        FSN_ENABLE_PRIVATE_REFLECTION;
    public:
        COMPONENT_DEFAULT(MeshRenderer)

        virtual void on_start() override;
        virtual void on_update(f32 delta) override;
        virtual void on_draw(RenderContext& ctx) override;

#if FSN_DEBUG_DRAW
        virtual void on_debug_draw(DebugDrawContext& ctx) override;
#endif

        [[API, EditorName("Model")]]
        AssetRef<Model> model_asset;

        std::vector<AssetRef<PbrMaterial>> materials {};

        virtual auto clone() -> Ref<Component> override;

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;

    private:
        [[API, NotifyFor("Model")]]
        void model_changed();
    };
}
