#pragma once
#include "Fussion/Assets/PbrMaterial.h"
#include "Fussion/Assets/AssetRef.h"
#include "Fussion/Scene/Component.h"
#include "Fussion/Assets/Model.h"

namespace Fussion {
    class MeshRenderer final : public Component {
    public:
        COMPONENT_DEFAULT(MeshRenderer)

        virtual void on_start() override;
        virtual void on_update(f32 delta) override;
        virtual void on_draw(RenderContext& ctx) override;

#if FSN_DEBUG_DRAW
        virtual void on_debug_draw(DebugDrawContext& ctx) override;
#endif

        AssetRef<Model> model;

        std::vector<AssetRef<PbrMaterial>> materials{};

        virtual auto clone() -> Ref<Component> override;

        virtual void serialize(Serializer& ctx) const override;
        virtual void deserialize(Deserializer& ctx) override;
    };
}
